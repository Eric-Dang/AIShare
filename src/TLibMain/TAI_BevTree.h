//-------------------------------------------------------------------------------------------------
// 	��������ʹ�ã����ʹ����ע�����������ߣ�����
//	*******************************************************
//	file path:	E:\GitHub\AIShare\src\TLibMain
//	file name:	TAI_BevTree.h
// 	Author:		Finney
// 	Blog:		AI����վ(http://www.aisharing.com/)
// 	A-Email:	finneytang@gmail.com
//	*******************************************************
// 	Modify:		Eric ���ע��,˵��
// 	E-EMail:	frederick.dang@gmail.com
// 	Git:		https://github.com/Eric-Dang/AIShare.git
//	*******************************************************
//	purpose:	
//-------------------------------------------------------------------------------------------------
#ifndef __TAI_BEVTREE_H__
#define __TAI_BEVTREE_H__

#include <string>
#include "TUtility_AnyData.h"

namespace TsiU{
	namespace AI{namespace BehaviorTree{
// �Լ����������� ��Ϊ�ӽڵ�洢ʹ�õ������飬Index��0-(k_BLimited_MaxChildNodeCnt-1)�����k_BLimited_MaxChildNodeCntҲ����Ч��Index 
#define k_BLimited_MaxChildNodeCnt              16		
#define k_BLimited_InvalidChildNodeIndex        k_BLimited_MaxChildNodeCnt

		enum E_ParallelFinishCondition
		{
			k_PFC_OR = 1,
			k_PFC_AND
		};

		enum BevRunningStatus
		{
			k_BRS_Executing					= 0,
			k_BRS_Finish					= 1,
			k_BRS_ERROR_Transition			= -1,
		};

		enum E_TerminalNodeStaus
		{
			k_TNS_Ready         = 1,
			k_TNS_Running       = 2,
			k_TNS_Finish        = 3,
		};

		typedef AnyData BevNodeInputParam;
		typedef AnyData BevNodeOutputParam;

		//-------------------------------------------------------------------------------------------------------------------------------------
		// �ڵ�Ԥ����
		class BevNodePrecondition
		{
		public:
			virtual bool ExternalCondition(const BevNodeInputParam& input) const = 0;
		};
		class BevNodePreconditionTRUE : public BevNodePrecondition
		{
		public:
			virtual bool ExternalCondition(const BevNodeInputParam& input) const{
				return true;
			}
		};
		class BevNodePreconditionFALSE : public BevNodePrecondition
		{
		public:
			virtual bool ExternalCondition(const BevNodeInputParam& input) const{
				return false;
			}
		};
		class BevNodePreconditionNOT : public BevNodePrecondition
		{
		public:
			BevNodePreconditionNOT(BevNodePrecondition* lhs)
				: m_lhs(lhs)
			{
				D_CHECK(m_lhs);
			}
			~BevNodePreconditionNOT(){
				D_SafeDelete(m_lhs);
			}
			virtual bool ExternalCondition(const BevNodeInputParam& input) const{
				return !m_lhs->ExternalCondition(input);
			}
		private:
			BevNodePrecondition* m_lhs;
		};
		class BevNodePreconditionAND : public BevNodePrecondition
		{
		public:
			BevNodePreconditionAND(BevNodePrecondition* lhs, BevNodePrecondition* rhs)
				: m_lhs(lhs)
				, m_rhs(rhs)
			{
				D_CHECK(m_lhs && m_rhs);
			}
			~BevNodePreconditionAND(){
				D_SafeDelete(m_lhs);
				D_SafeDelete(m_rhs);
			}
			virtual bool ExternalCondition(const BevNodeInputParam& input) const{
				return m_lhs->ExternalCondition(input) && m_rhs->ExternalCondition(input);
			}
		private:
			BevNodePrecondition* m_lhs;
			BevNodePrecondition* m_rhs;
		};
		class BevNodePreconditionOR : public BevNodePrecondition
		{
		public:
			BevNodePreconditionOR(BevNodePrecondition* lhs, BevNodePrecondition* rhs)
				: m_lhs(lhs)
				, m_rhs(rhs)
			{
				D_CHECK(m_lhs && m_rhs);
			}
			~BevNodePreconditionOR(){
				D_SafeDelete(m_lhs);
				D_SafeDelete(m_rhs);
			}
			virtual bool ExternalCondition(const BevNodeInputParam& input) const{
				return m_lhs->ExternalCondition(input) || m_rhs->ExternalCondition(input);
			}
		private:
			BevNodePrecondition* m_lhs;
			BevNodePrecondition* m_rhs;
		};
		class BevNodePreconditionXOR : public BevNodePrecondition
		{
		public:
			BevNodePreconditionXOR(BevNodePrecondition* lhs, BevNodePrecondition* rhs)
				: m_lhs(lhs)
				, m_rhs(rhs)
			{
				D_CHECK(m_lhs && m_rhs);
			}
			~BevNodePreconditionXOR(){
				D_SafeDelete(m_lhs);
				D_SafeDelete(m_rhs);
			}
			virtual bool ExternalCondition(const BevNodeInputParam& input) const{
				return m_lhs->ExternalCondition(input) ^ m_rhs->ExternalCondition(input);
			}
		private:
			BevNodePrecondition* m_lhs;
			BevNodePrecondition* m_rhs;
		};
		//-------------------------------------------------------------------------------------------------------------------------------------
		// ��Ϊ���ڵ㸸��
		class BevNode
		{
		public:
			BevNode(BevNode* _o_ParentNode, BevNodePrecondition* _o_NodeScript = NULL)
				: mul_ChildNodeCount(0)
				, mz_DebugName("UNNAMED")
				, mo_ActiveNode(NULL)
				, mo_LastActiveNode(NULL)
				, mo_NodePrecondition(NULL)
			{
				for(int i = 0; i < k_BLimited_MaxChildNodeCnt; ++i)
					mao_ChildNodeList[i] = NULL;

				_SetParentNode(_o_ParentNode);
				SetNodePrecondition(_o_NodeScript);
			}
			virtual ~BevNode()
			{
				for(unsigned int i = 0; i < mul_ChildNodeCount; ++i)
				{
					D_SafeDelete(mao_ChildNodeList[i]);
				}
				D_SafeDelete(mo_NodePrecondition);
			}
			// ���ڵ��Ƿ����ִ�У� ��֤��ǰ�������Լ��ڵ����������
			bool Evaluate(const BevNodeInputParam& input)
			{
				return (mo_NodePrecondition == NULL || mo_NodePrecondition->ExternalCondition(input)) && _DoEvaluate(input);
			}
			// TODO: _DoTransition �������������ע��
			// ѡ��ڵ���ִ����һ���ӽڵ���л��������ڵ�ʱ����Ҫ��ִ��
			void Transition(const BevNodeInputParam& input)
			{
				_DoTransition(input);
			}
			BevRunningStatus Tick(const BevNodeInputParam& input, BevNodeOutputParam& output)
			{
				return _DoTick(input, output);
			}
			//---------------------------------------------------------------
			// ����ӽڵ�
			BevNode& AddChildNode(BevNode* _o_ChildNode)
			{
				if(mul_ChildNodeCount == k_BLimited_MaxChildNodeCnt)
				{
					D_Output("The number of child nodes is up to 16");
					D_CHECK(0);
					return (*this);
				}
				mao_ChildNodeList[mul_ChildNodeCount] = _o_ChildNode;
				++mul_ChildNodeCount;
				return (*this);
			}
			// ���ýڵ�ǰ�������� ��������
			BevNode& SetNodePrecondition(BevNodePrecondition* _o_NodePrecondition)
			{
				if(mo_NodePrecondition != _o_NodePrecondition)
				{
					if(mo_NodePrecondition)
						delete mo_NodePrecondition;

					mo_NodePrecondition = _o_NodePrecondition;
				}
				return (*this);
			}
			// ���ýڵ��ʾ
			BevNode& SetDebugName(const char* _debugName)
			{
				mz_DebugName = _debugName;
				return (*this);
			}
			// TODO:��ȡ�ϴμ���Ľڵ�
			const BevNode* oGetLastActiveNode() const
			{
				return mo_LastActiveNode;
			}
			// ���õ�ǰ������Ľڵ�
			// 1. �����ϴ�ִ�еĽڵ�
			// 2. ���õ�ǰ����Ľڵ�
			// 3. ͬʱ���ø��ڵ�ĵ�ִ�нڵ㣬 ���Root�ڵ㱣���˵�ǰִ�еĽڵ㣬�´ξͲ�����Ҫ���� TODO
			void SetActiveNode(BevNode* _o_Node)
			{
				mo_LastActiveNode = mo_ActiveNode;
				mo_ActiveNode = _o_Node;
				if(mo_ParentNode != NULL)
					mo_ParentNode->SetActiveNode(_o_Node);
			}
			// ��ȡ�ڵ��ʾ
			const char* GetDebugName() const
			{
				return mz_DebugName.c_str();
			}
		protected:
			//--------------------------------------------------------------
			// virtual function
			//--------------------------------------------------------------
			// �����Ƿ���Ҫִ�иýڵ�
			virtual bool _DoEvaluate(const BevNodeInputParam& input)
			{
				return true;
			}

			// TODO: �ڱ任�������ڵ�ǰ����Ҫִ�еĶ����� Ҳ�ɳ�Ϊ�뿪�ڵ���Ҫִ�еĶ���
			virtual void _DoTransition(const BevNodeInputParam& input)
			{
			}
			// �ڵ�ѭ��ִ�е����
			virtual BevRunningStatus _DoTick(const BevNodeInputParam& input, BevNodeOutputParam& output)
			{
				return k_BRS_Finish;
			}
		protected:
			// ���ø��ڵ�
			void _SetParentNode(BevNode* _o_ParentNode)
			{
				mo_ParentNode = _o_ParentNode;
			}
			// ���Index����Ч��
			bool _bCheckIndex(u32 _ui_Index) const
			{
				return _ui_Index >= 0 && _ui_Index < mul_ChildNodeCount;
			}
		protected:

			// �ӽڵ�
			BevNode*                mao_ChildNodeList[k_BLimited_MaxChildNodeCnt];

			// �ӽڵ�����
			u32						mul_ChildNodeCount;

			// ���ڵ�
			BevNode*                mo_ParentNode;

			// TODO: ��ǰ����Ľڵ㣬Ŀ���Ǽ��ٱ���
			BevNode*                mo_ActiveNode;

			// TODO: �ϴμ���Ľڵ�,��ʱ����֪��ʲô����
			BevNode*				mo_LastActiveNode;

			// �ڵ�ִ�е�Ԥ������
			BevNodePrecondition*    mo_NodePrecondition;

			// �ڵ��ʾ��
			std::string				mz_DebugName;
		};

		// �������ȼ���selector�ڵ㣬 �������������ȼ�
		class BevNodePrioritySelector : public BevNode
		{
		public:
			BevNodePrioritySelector(BevNode* _o_ParentNode, BevNodePrecondition* _o_NodePrecondition = NULL)
				: BevNode(_o_ParentNode, _o_NodePrecondition)
				, mui_LastSelectIndex(k_BLimited_InvalidChildNodeIndex)
				, mui_CurrentSelectIndex(k_BLimited_InvalidChildNodeIndex)
			{}

			// �������е��ӽڵ㣬����Ƿ��п���ִ�еĽڵ�, mui_CurrentSelectIndex��Ҫִ�еĽڵ�Index
			virtual bool _DoEvaluate(const BevNodeInputParam& input);
			// TODO: �ڱ任�������ڵ�ǰ����Ҫִ�еĶ����� Ҳ�ɳ�Ϊ�뿪�ڵ���Ҫִ�еĶ���
			virtual void _DoTransition(const BevNodeInputParam& input);
			// ִ�нӿ�
			virtual BevRunningStatus _DoTick(const BevNodeInputParam& input, BevNodeOutputParam& output);

		protected:
			// ��ǰTick��Ҫִ�еĽڵ�Index
			u32 mui_CurrentSelectIndex;
			// �ϴ�ִ�еĽڵ�Index
			u32 mui_LastSelectIndex;
		};

		// û�����ȼ���ѡ��ڵ�		
		class BevNodeNonePrioritySelector : public BevNodePrioritySelector
		{
		public:
			BevNodeNonePrioritySelector(BevNode* _o_ParentNode, BevNodePrecondition* _o_NodePrecondition = NULL)
				: BevNodePrioritySelector(_o_ParentNode, _o_NodePrecondition)
			{}
			virtual bool _DoEvaluate(const BevNodeInputParam& input);
		};

		// ˳��ڵ�
		class BevNodeSequence : public BevNode
		{
		public:
			BevNodeSequence(BevNode* _o_ParentNode, BevNodePrecondition* _o_NodePrecondition = NULL)
				: BevNode(_o_ParentNode, _o_NodePrecondition)
				, mui_CurrentNodeIndex(k_BLimited_InvalidChildNodeIndex)
			{}
			virtual bool _DoEvaluate(const BevNodeInputParam& input);
			virtual void _DoTransition(const BevNodeInputParam& input);
			virtual BevRunningStatus _DoTick(const BevNodeInputParam& input, BevNodeOutputParam& output);

		private:
			// ��ǰִ�е��Ľڵ��Index
			u32 mui_CurrentNodeIndex;
		};

		// TODO:�ն˽ڵ� ���ã�����
		// ���м򵥵�״̬��
		class BevNodeTerminal : public BevNode
		{
		public:
			BevNodeTerminal(BevNode* _o_ParentNode, BevNodePrecondition* _o_NodePrecondition = NULL)
				: BevNode(_o_ParentNode, _o_NodePrecondition)
				, me_Status(k_TNS_Ready)
				, mb_NeedExit(false)
			{}
			virtual void _DoTransition(const BevNodeInputParam& input);
			// ���ݵ�ǰ�ڵ��״ִ̬�ж�Ӧ�Ĵ���
			// k_TNS_Ready:		���õ�ǰ����ڵ㣬 ��ת״̬��Running, ��������ִ��Running����
			// k_TNS_Running:	ִ��_DoExecute,���ݷ������趨����״̬, �������ΪTNS_Finish��������ֱ��ִ��Finish����
			// k_TNS_Finish:	ִ��Finish������ ������սڵ��������
			virtual BevRunningStatus _DoTick(const BevNodeInputParam& input, BevNodeOutputParam& output);

		protected:
			// _DoEnter �� _DoExit ������Գ��֣� �����Enter�ڽڵ�ִ�����ǰ������Exit, ���򶼲�����
			// Init
			virtual void				_DoEnter(const BevNodeInputParam& input)								{}
			// Runing ִ��
			virtual BevRunningStatus	_DoExecute(const BevNodeInputParam& input, BevNodeOutputParam& output)	{ return k_BRS_Finish;}
			// Leave
			virtual void				_DoExit(const BevNodeInputParam& input, BevRunningStatus _ui_ExitID)	{}

		private:
			E_TerminalNodeStaus me_Status;
			bool                mb_NeedExit;
		};

		// �����ڵ�
		class BevNodeParallel : public BevNode
		{
		public:
			BevNodeParallel(BevNode* _o_ParentNode, BevNodePrecondition* _o_NodePrecondition = NULL)
				: BevNode(_o_ParentNode, _o_NodePrecondition)
				, me_FinishCondition(k_PFC_OR)
			{
				for(unsigned int i = 0; i < k_BLimited_MaxChildNodeCnt; ++i)
					mab_ChildNodeStatus[i] = k_BRS_Executing;
			}
			// ���������Ҫִ�еĽڵ��Ƿ񶼿���ִ��
			virtual bool _DoEvaluate(const BevNodeInputParam& input);
			// �������������ӽڵ�����У� ͬʱִ�������ӽڵ��Transition
			virtual void _DoTransition(const BevNodeInputParam& input);
			virtual BevRunningStatus _DoTick(const BevNodeInputParam& input, BevNodeOutputParam& output);

			BevNodeParallel& SetFinishCondition(E_ParallelFinishCondition _e_Condition);

		private:
			// ���ؽ������ʽ And or Or
			E_ParallelFinishCondition me_FinishCondition;
			// �ӽڵ������״̬
			BevRunningStatus		  mab_ChildNodeStatus[k_BLimited_MaxChildNodeCnt];
		};

		// ѭ���ڵ� �ýڵ�ֻ������һ���ӽڵ�
		class BevNodeLoop : public BevNode
		{
		public:
			static const int kInfiniteLoop = -1;

		public:
			BevNodeLoop(BevNode* _o_ParentNode, BevNodePrecondition* _o_NodePrecondition = NULL, int _i_LoopCnt = kInfiniteLoop)
				: BevNode(_o_ParentNode, _o_NodePrecondition)
				, mi_LoopCount(_i_LoopCnt)
				, mi_CurrentCount(0)
			{}
			virtual bool _DoEvaluate(const BevNodeInputParam& input);
			virtual void _DoTransition(const BevNodeInputParam& input);
			virtual BevRunningStatus _DoTick(const BevNodeInputParam& input, BevNodeOutputParam& output);

		private:
			// ѭ���Ĵ���
			int mi_LoopCount;
			// ��ǰ�Ѿ�ִ�еĴ���
			int mi_CurrentCount;
		};

		class BevNodeFactory
		{
		public:
			static BevNode& oCreateParallelNode(BevNode* _o_Parent, E_ParallelFinishCondition _e_Condition, const char* _debugName)
			{
				BevNodeParallel* pReturn = new BevNodeParallel(_o_Parent);
				pReturn->SetFinishCondition(_e_Condition);
				oCreateNodeCommon(pReturn, _o_Parent, _debugName);
				return (*pReturn);
			}
			static BevNode& oCreatePrioritySelectorNode(BevNode* _o_Parent, const char* _debugName)
			{
				BevNodePrioritySelector* pReturn = new BevNodePrioritySelector(_o_Parent);
				oCreateNodeCommon(pReturn, _o_Parent, _debugName);
				return (*pReturn);
			}
			static BevNode& oCreateNonePrioritySelectorNode(BevNode* _o_Parent, const char* _debugName)
			{
				BevNodeNonePrioritySelector* pReturn = new BevNodeNonePrioritySelector(_o_Parent);
				oCreateNodeCommon(pReturn, _o_Parent, _debugName);
				return (*pReturn);
			}
			static BevNode& oCreateSequenceNode(BevNode* _o_Parent, const char* _debugName)
			{
				BevNodeSequence* pReturn = new BevNodeSequence(_o_Parent);
				oCreateNodeCommon(pReturn, _o_Parent, _debugName);
				return (*pReturn);
			}
			static BevNode& oCreateLoopNode(BevNode* _o_Parent, const char* _debugName, int _i_LoopCount)
			{
				BevNodeLoop* pReturn = new BevNodeLoop(_o_Parent, NULL, _i_LoopCount);
				oCreateNodeCommon(pReturn, _o_Parent, _debugName);
				return (*pReturn);
			}
			// ����Ҷ�ӽڵ� class T : public BevNodeTerminal
			template<typename T>
			static BevNode& oCreateTeminalNode(BevNode* _o_Parent, const char* _debugName)
			{
				BevNodeTerminal* pReturn = new T(_o_Parent);
				oCreateNodeCommon(pReturn, _o_Parent, _debugName);
				return (*pReturn);
			}
		private:
			static void oCreateNodeCommon(BevNode* _o_Me, BevNode* _o_Parent, const char* _debugName)
			{
				if(_o_Parent)
					_o_Parent->AddChildNode(_o_Me);
				_o_Me->SetDebugName(_debugName);
			}
		};
	}}}

#endif
