namespace NWA_OBDD {
	NWAOBDDNodeHandle MkCNot2Node(unsigned int level, unsigned int n, long int controller, long int controlled) {
		std::string p = std::to_string(level) + ";" + std::to_string(controller) + ";" + std::to_string(controlled);
		if (cnot_hashMap.find(p) != cnot_hashMap.end()){
			return cnot_hashMap[p];
		}	

		NWAOBDDInternalNode *g;

		if (level == 1)
		{
			pseudoCFLOBDDBase c;
			if (controller == 0)
			{
				c.numBConnections = 2;
				c.numExits = 3;
				c.bconn[0] = intpair(0, 1);
				c.bconn[1] = intpair(1, 2);
			}
			else if (controlled == 0)
			{
				c.numBConnections = 2;
				c.numExits = 2;
				c.bconn[0] = intpair(0, 1);
				c.bconn[1] = intpair(1, 0);
			}
			g = c.toNWA();
		}
		else
		{
			pseudoCFLOBDDInternal c(level);
			if (controller < n/2 && controlled < n/2 && controlled >= 0 && controller >= 0)
			{
				// Case 1: Both in A CFLConnection
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				auto aa = MkCNot2Node(level-1, n/2, controller, controlled);
				c.AConnection = CFLConnection(aa, m01);
				c.numBConnections = 2;
				c.BConnection = new CFLConnection[2];
				auto Id = MkIdNode(level - 1);
				c.BConnection[0] = CFLConnection(Id, m01);
				ReturnMapHandle<int> m1;
				m1.AddToEnd(1); m1.Canonicalize();
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.numExits = 2;
			}
			else if (controller >= n/2 && controlled >= n/2 && controller >= 0 && controlled >= 0)
			{
				// Case 2: Both in B CFLConnection
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				auto Id = MkIdNode(level - 1);
				c.AConnection = CFLConnection(Id, m01);
				c.numBConnections = 2;
				c.BConnection = new CFLConnection[2];
				auto bb = MkCNot2Node(level-1, n/2, controller - n/2, controlled - n/2);
				c.BConnection[0] = CFLConnection(bb, m01);
				ReturnMapHandle<int> m1;
				m1.AddToEnd(1); m1.Canonicalize();
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.numExits = 2;
			}
			else if (controller < n/2 && controlled >= n/2 && controller >= 0 && controlled >= 0)
			{
				// Case 3: controller in A and controlled in B
				ReturnMapHandle<int> m012;
				m012.AddToEnd(0); m012.AddToEnd(1); m012.AddToEnd(2); m012.Canonicalize();
				auto aa = MkCNot2Node(level-1, n/2, controller, -1);
				c.AConnection = CFLConnection(aa, m012);
				c.numBConnections = 3;
				c.BConnection = new CFLConnection[3];
				ReturnMapHandle<int> m01, m1, m10;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				m10.AddToEnd(1); m10.AddToEnd(0); m10.Canonicalize();
				auto bb = MkCNot2Node(level-1, n/2, -1, controlled - n/2);
				auto Id = MkIdNode(level - 1);
				c.BConnection[0] = CFLConnection(Id, m01);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.BConnection[2] = CFLConnection(bb, m10);
				c.numExits = 2;
			}
			else if (controlled == -1 && controller < n/2 && controller >= 0)
			{
				// Case 4: controller in A and controlled == -1
				ReturnMapHandle<int> m012;
				m012.AddToEnd(0); m012.AddToEnd(1); m012.AddToEnd(2); m012.Canonicalize();
				auto aa = MkCNot2Node(level-1, n/2, controller, -1);
				c.AConnection = CFLConnection(aa, m012);
				c.numBConnections = 3;
				c.BConnection = new CFLConnection[c.numBConnections];
				ReturnMapHandle<int> m01, m1, m21;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				m21.AddToEnd(2); m21.AddToEnd(1); m21.Canonicalize();
				auto Id = MkIdNode(level - 1);
				c.BConnection[0] = CFLConnection(Id, m01);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.BConnection[2] = CFLConnection(Id, m21);
				c.numExits = 3;
			}
			else if (controlled == -1 && controller >= n/2)
			{
				// Case 5: controller in B and controlled == -1
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				auto Id = MkIdNode(level - 1);
				c.AConnection = CFLConnection(Id, m01);
				c.numBConnections = 2;
				c.BConnection = new CFLConnection[2];
				auto bb = MkCNot2Node(level-1, n/2, controller - n/2, -1);
				ReturnMapHandle<int> m012, m1;
				m012.AddToEnd(0); m012.AddToEnd(1); m012.AddToEnd(2); m012.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				c.BConnection[0] = CFLConnection(bb, m012);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.numExits = 3;
			}
			else if (controller == -1 && controlled >= 0 && controlled < n/2)
			{
				// Case 6: controller == -1 and controlled in A
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				auto aa = MkCNot2Node(level-1, n/2, -1, controlled);
				c.AConnection = CFLConnection(aa, m01);
				c.numBConnections = 2;
				c.BConnection = new CFLConnection[2];
				ReturnMapHandle<int> m0, m10;
				m0.AddToEnd(0); m0.Canonicalize();
				m10.AddToEnd(1); m10.AddToEnd(0); m10.Canonicalize();
				c.BConnection[0] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m0);
				auto Id = MkIdNode(level - 1);
				c.BConnection[1] = CFLConnection(Id, m10);
				c.numExits = 2;
			}
			else if (controller == -1 && controlled >= n/2)
			{
				// Case 7: controller == -1 and controlled in B
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				auto Id = MkIdNode(level - 1);
				c.AConnection = CFLConnection(Id, m01);
				c.numBConnections = 2;
				c.BConnection = new CFLConnection[2];
				auto bb = MkCNot2Node(level-1, n/2, -1, controlled - n/2);
				ReturnMapHandle<int> m0;
				m0.AddToEnd(0); m0.Canonicalize();
				c.BConnection[0] = CFLConnection(bb, m01);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m0);
				c.numExits = 2;
			}
			g = c.toNWA();
		}

	#ifdef PATH_COUNTING_ENABLED
			g->InstallPathCounts();
	#endif
		NWAOBDDNodeHandle gHandle = NWAOBDDNodeHandle(g);
		cnot_hashMap.insert(std::make_pair(p, gHandle));
		return gHandle;
	}

	NWAOBDDNodeHandle MkCCNotNode(unsigned int level, unsigned int n, long int controller1, long int controller2, long int controlled)
	{
		std::string p = std::to_string(level) + ";" + std::to_string(controller1) + ";" + std::to_string(controller2) + ";" + std::to_string(controlled);
		if (ccnot_hashMap.find(p) != ccnot_hashMap.end()){
			return ccnot_hashMap[p];
		}	

		NWAOBDDInternalNode *g;

		if (level == 1)
		{
			pseudoCFLOBDDBase c;
			if (controller1 == 0 || controller2 == 0)
			{
				c.numBConnections = 2;
				c.bconn[0] = intpair(0, 1);
				c.bconn[1] = intpair(1, 2);
				c.numExits = 3;
			}
			else if (controlled == 0)
			{
				c.numBConnections = 2;
				c.bconn[0] = intpair(0, 1);
				c.bconn[1] = intpair(1, 0);
				c.numExits = 2;
			}
			g = c.toNWA();
		}
		else
		{
			pseudoCFLOBDDInternal c(level);
			if (controller1 < n/2 && controller2 < n/2 && controlled < n/2 && controlled >= 0 && controller1 >= 0 && controller2 >= 0)
			{
				// Case 1: CR1, CR2 and CD in A CFLConnection
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				auto aa = MkCCNotNode(level-1, n/2, controller1, controller2, controlled);
				c.AConnection = CFLConnection(aa, m01);
				c.numBConnections = 2;
				c.BConnection = new CFLConnection[2];
				auto Id = MkIdNode(level - 1);
				c.BConnection[0] = CFLConnection(Id, m01);
				ReturnMapHandle<int> m1;
				m1.AddToEnd(1); m1.Canonicalize();
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.numExits = 2;
			}
			else if (controller1 >= n/2 && controller2 >= n/2 && controlled >= n/2 && controller1 >= 0 && controller2 >= 0 && controlled >= 0)
			{
				// Case 2: CR1, CR2 and CD in B CFLConnection
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				auto Id = MkIdNode(level - 1);
				c.AConnection = CFLConnection(Id, m01);
				c.numBConnections = 2;
				c.BConnection = new CFLConnection[2];
				auto bb = MkCCNotNode(level-1, n/2, controller1 - n/2, controller2 - n/2, controlled - n/2);
				c.BConnection[0] = CFLConnection(bb, m01);
				ReturnMapHandle<int> m1;
				m1.AddToEnd(1); m1.Canonicalize();
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.numExits = 2;
			}
			else if (controller1 < n/2 && controller2 >= n/2 && controlled >= n/2 && controller1 >= 0 && controlled >= 0 && controller2 >= 0)
			{
				// Case 3: CR1 in A, CR2 and CD in B
				ReturnMapHandle<int> m012;
				m012.AddToEnd(0); m012.AddToEnd(1); m012.AddToEnd(2); m012.Canonicalize();
				auto aa = MkCCNotNode(level-1, n/2, controller1, -1, -1);
				c.AConnection = CFLConnection(aa, m012);
				c.numBConnections = 3;
				c.BConnection = new CFLConnection[3];
				ReturnMapHandle<int> m01, m1, m10;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				m10.AddToEnd(1); m10.AddToEnd(0); m10.Canonicalize();
				auto bb = MkCCNotNode(level-1, n/2, -1, controller2 - n/2, controlled - n/2);
				auto Id = MkIdNode(level - 1);
				c.BConnection[0] = CFLConnection(Id, m01);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.BConnection[2] = CFLConnection(bb, m01);
				c.numExits = 2;
			}
			else if (controlled == -1 && controller1 < n/2 && controller1 >= 0 && controller2 == -1)
			{
				// Case 4: CR1 in A and CR2 == -1 and CD == -1
				ReturnMapHandle<int> m012;
				m012.AddToEnd(0); m012.AddToEnd(1); m012.AddToEnd(2); m012.Canonicalize();
				auto aa = MkCCNotNode(level-1, n/2, controller1, -1, -1);
				c.AConnection = CFLConnection(aa, m012);
				c.numBConnections = 3;
				c.BConnection = new CFLConnection[c.numBConnections];
				ReturnMapHandle<int> m01, m1, m21;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				m21.AddToEnd(2); m21.AddToEnd(1); m21.Canonicalize();
				auto Id = MkIdNode(level - 1);
				c.BConnection[0] = CFLConnection(Id, m01);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.BConnection[2] = CFLConnection(Id, m21);
				c.numExits = 3;
			}
			else if (controlled == -1 && controller1 >= n/2 && controller2 == -1)
			{
				// Case 5: CR1 in B and CR2 == -1 and CD == -1
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				auto Id = MkIdNode(level - 1);
				c.AConnection = CFLConnection(Id, m01);
				c.numBConnections = 2;
				c.BConnection = new CFLConnection[2];
				auto bb = MkCCNotNode(level-1, n/2, controller1 - n/2, -1, -1);
				ReturnMapHandle<int> m012, m1;
				m012.AddToEnd(0); m012.AddToEnd(1); m012.AddToEnd(2); m012.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				c.BConnection[0] = CFLConnection(bb, m012);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.numExits = 3;
			}
			else if (controller1 == -1 && controlled >= 0 && controlled < n/2 && controller2 == -1)
			{
				// Case 6: controller == -1 and controlled in A
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				auto aa = MkCCNotNode(level-1, n/2, -1, -1, controlled);
				c.AConnection = CFLConnection(aa, m01);
				c.numBConnections = 2;
				c.BConnection = new CFLConnection[2];
				ReturnMapHandle<int> m0, m10;
				m0.AddToEnd(0); m0.Canonicalize();
				m10.AddToEnd(1); m10.AddToEnd(0); m10.Canonicalize();
				c.BConnection[0] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m0);
				auto Id = MkIdNode(level - 1);
				c.BConnection[1] = CFLConnection(Id, m10);
				c.numExits = 2;
			}
			else if (controller1 == -1 && controlled >= n/2 && controller2 == -1)
			{
				// Case 7: controller == -1 and controlled in B
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				auto Id = MkIdNode(level - 1);
				c.AConnection = CFLConnection(Id, m01);
				c.numBConnections = 2;
				c.BConnection = new CFLConnection[2];
				auto bb = MkCCNotNode(level-1, n/2, -1, -1, controlled - n/2);
				ReturnMapHandle<int> m0;
				m0.AddToEnd(0); m0.Canonicalize();
				c.BConnection[0] = CFLConnection(bb, m01);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m0);
				c.numExits = 2;
			}
			else if (controller1 < n/2 && controller2 < n/2 && controlled >= n/2 && controller1 >= 0 && controlled >= 0 && controller2 >= 0)
			{
				// Case 8: CR1 and CR2 in A, CD in B
				ReturnMapHandle<int> m012;
				m012.AddToEnd(0); m012.AddToEnd(1); m012.AddToEnd(2); m012.Canonicalize();
				auto aa = MkCCNotNode(level-1, n/2, controller1, controller2, -1);
				c.AConnection = CFLConnection(aa, m012);
				c.numBConnections = 3;
				c.BConnection = new CFLConnection[3];
				ReturnMapHandle<int> m01, m1, m10;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				m10.AddToEnd(1); m10.AddToEnd(0); m10.Canonicalize();
				auto bb = MkCCNotNode(level-1, n/2, -1, -1, controlled - n/2);
				auto Id = MkIdNode(level - 1);
				c.BConnection[0] = CFLConnection(Id, m01);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.BConnection[2] = CFLConnection(bb, m10);
				c.numExits = 2;
			}
			else if (controller1 < n/2 && controller2 < n/2 && controlled == -1 && controller1 >= 0 && controller2 >= 0)
			{
				// Case 9: CR1 and CR2 in A, CD == -1
				ReturnMapHandle<int> m012;
				m012.AddToEnd(0); m012.AddToEnd(1); m012.AddToEnd(2); m012.Canonicalize();
				auto aa = MkCCNotNode(level-1, n/2, controller1, controller2, -1);
				c.AConnection = CFLConnection(aa, m012);
				c.numBConnections = 3;
				c.BConnection = new CFLConnection[3];
				ReturnMapHandle<int> m01, m1, m21;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				m21.AddToEnd(2); m21.AddToEnd(1); m21.Canonicalize();
				auto Id = MkIdNode(level - 1);
				c.BConnection[0] = CFLConnection(Id, m01);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.BConnection[2] = CFLConnection(Id, m21);
				c.numExits = 3;
			}
			else if (controller1 >= n/2 && controller2 >= n/2 && controlled == -1 && controller1 >= 0 && controller2 >= 0)
			{
				// Case 10: CR1 and CR2 in B, CD == -1
				auto Id = MkIdNode(level - 1);
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				c.AConnection = CFLConnection(Id, m01);
				c.numBConnections = 2;
				c.BConnection = new CFLConnection[2];
				ReturnMapHandle<int> m012, m1;
				m012.AddToEnd(0); m012.AddToEnd(1); m012.AddToEnd(2); m012.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				auto bb = MkCCNotNode(level-1, n/2, controller1 - n/2, controller2 - n/2, -1);
				c.BConnection[0] = CFLConnection(bb, m012);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.numExits = 3;
			}
			else if (controller1 < n/2 && controller2 >= n/2 && controlled == -1 && controller1 >= 0 && controller2 >= 0)
			{
				// Case 11: CR1 in A, CR2 in B, CD == -1
				auto Id = MkIdNode(level - 1);
				auto aa = MkCCNotNode(level-1, n/2, controller1, -1, -1);
				ReturnMapHandle<int> m012;
				m012.AddToEnd(0); m012.AddToEnd(1); m012.AddToEnd(2); m012.Canonicalize();
				c.AConnection = CFLConnection(aa, m012);
				c.numBConnections = 3;
				c.BConnection = new CFLConnection[3];
				ReturnMapHandle<int> m01, m1;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				auto bb = MkCCNotNode(level-1, n/2, -1, controller2 - n/2, -1);
				c.BConnection[0] = CFLConnection(Id, m01);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.BConnection[2] = CFLConnection(bb, m012);
				c.numExits = 3;
			}
			else if (controller1 == -1 && controller2 < n/2 && controlled == -1 && controller2 >= 0)
			{
				// Case 12: CR1 == -1, CR2 in A, CD == -1
				auto Id = MkIdNode(level - 1);
				auto aa = MkCCNotNode(level-1, n/2, -1, controller2, -1);
				ReturnMapHandle<int> m012;
				m012.AddToEnd(0); m012.AddToEnd(1); m012.AddToEnd(2); m012.Canonicalize();
				c.AConnection = CFLConnection(aa, m012);
				c.numBConnections = 3;
				c.BConnection = new CFLConnection[3];
				ReturnMapHandle<int> m01, m1, m21;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				m21.AddToEnd(2); m21.AddToEnd(1); m21.Canonicalize();
				c.BConnection[0] = CFLConnection(Id, m01);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.BConnection[2] = CFLConnection(Id, m21);
				c.numExits = 3;
			}
			else if (controller1 == -1 && controller2 >= n/2 && controlled == -1 && controller2 >= 0)
			{
				// Case 12: CR1 == -1, CR2 in B, CD == -1
				auto Id = MkIdNode(level - 1);
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				c.AConnection = CFLConnection(Id, m01);
				c.numBConnections = 2;
				c.BConnection = new CFLConnection[2];
				ReturnMapHandle<int> m012, m1;
				m012.AddToEnd(0); m012.AddToEnd(1); m012.AddToEnd(2); m012.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				auto bb = MkCCNotNode(level-1, n/2, -1, controller2 - n/2, -1);
				c.BConnection[0] = CFLConnection(bb, m012);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.numExits = 3;
			}
			else if (controller1 == -1 && controller2 < n/2 && controlled < n/2 && controller2 >= 0 && controlled >= 0)
			{
				// Case 13: CR1 == -1, CR2 and CD in A
				auto Id = MkIdNode(level - 1);
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				auto aa = MkCCNotNode(level-1, n/2, -1, controller2, controlled);
				c.AConnection = CFLConnection(aa, m01);
				c.numBConnections = 2;
				c.BConnection = new CFLConnection[2];
				ReturnMapHandle<int> m1;
				m1.AddToEnd(1); m1.Canonicalize();
				c.BConnection[0] = CFLConnection(Id, m01);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.numExits = 2;
			}
			else if (controller1 == -1 && controller2 >= n/2 && controlled >= n/2 && controller2 >= 0 && controlled >= 0)
			{
				// Case 14: CR1 == -1, CR2 and CD in B
				auto Id = MkIdNode(level - 1);
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				c.AConnection = CFLConnection(Id, m01);
				c.numBConnections = 2;
				c.BConnection = new CFLConnection[2];
				ReturnMapHandle<int> m1;
				m1.AddToEnd(1); m1.Canonicalize();
				auto bb = MkCCNotNode(level-1, n/2, -1, controller2 - n/2, controlled - n/2);
				c.BConnection[0] = CFLConnection(bb, m01);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.numExits = 2;
			}
			else if (controller1 == -1 && controller2 < n/2 && controlled >= n/2 && controller2 >= 0 && controlled >= 0)
			{
				// Case 15: CR1 == -1, CR2 in A and CD in B
				auto Id = MkIdNode(level - 1);
				ReturnMapHandle<int> m012;
				m012.AddToEnd(0); m012.AddToEnd(1); m012.AddToEnd(2); m012.Canonicalize();
				auto aa = MkCCNotNode(level-1, n/2, -1, controller2, -1);
				c.AConnection = CFLConnection(aa, m012);
				c.numBConnections = 3;
				c.BConnection = new CFLConnection[3];
				ReturnMapHandle<int> m01, m1, m10;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				m10.AddToEnd(1); m10.AddToEnd(0); m10.Canonicalize();
				auto bb = MkCCNotNode(level-1, n/2, -1, -1, controlled - n/2);
				c.BConnection[0] = CFLConnection(Id, m01);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.BConnection[2] = CFLConnection(bb, m10);
				c.numExits = 2;
			}
			g = c.toNWA();
		}

#ifdef PATH_COUNTING_ENABLED
		g->InstallPathCounts();
#endif
		NWAOBDDNodeHandle gHandle = NWAOBDDNodeHandle(g);
		ccnot_hashMap.insert(std::make_pair(p, gHandle));
		return gHandle;
	}


	NWAOBDDNodeHandle MkSwapNode(unsigned int level, long int controller, long int controlled, int case_num) {
		std::string p = std::to_string(level) + ";" + std::to_string(controller) + ";" + std::to_string(controlled) + ";" + std::to_string(case_num);
		if (swap_hashMap.find(p) != swap_hashMap.end()){
			return swap_hashMap[p];
		}
		NWAOBDDInternalNode *g = new NWAOBDDInternalNode(level);
		if (level == 1)
		{
			pseudoCFLOBDDBase c;
			// std::cout << "CaseBase" << std::endl;
			if (case_num == -1)
			{
				assert(controlled == -1);
				c.numBConnections = 2;
				c.bconn[0] = intpair(0, 1);
				c.bconn[1] = intpair(2, 3);
				c.numExits = 4;
			}
			else if (case_num == 0)
			{
				// [[1 0] [0 0]]
				assert(controller == -1);
				c.numBConnections = 2;
				c.bconn[0] = intpair(0, 1);
				c.bconn[1] = intpair(1, 1);
				c.numExits = 2;
			}
			else if (case_num == 1)
			{
				// [[0 1] [0 0]]
				assert(controller == -1);
				c.numBConnections = 2;
				c.bconn[0] = intpair(0, 1);
				c.bconn[1] = intpair(0, 0);
				c.numExits = 2;	
			}
			else if (case_num == 2)
			{
				// [[0 0] [1 0]]
				assert(controller == -1);
				c.numBConnections = 2;
				c.bconn[0] = intpair(0, 0);
				c.bconn[1] = intpair(1, 0);
				c.numExits = 2;	
			}
			else if (case_num == 3)
			{
				// [[0 0] [0 1]]
				assert(controller == -1);
				c.numBConnections = 2;
				c.bconn[0] = intpair(0, 0);
				c.bconn[1] = intpair(0, 1);
				c.numExits = 2;	
			}
			g = c.toNWA();
		}
		else if (level == 2 && controller == 0 && controlled == 1)
		{
			pseudoCFLOBDDInternal c(level);

			NWAOBDDNodeHandle atmp = MkSwapNode(level - 1, controller, -1, case_num);
			ReturnMapHandle<int> m0123;
			m0123.AddToEnd(0); m0123.AddToEnd(1); m0123.AddToEnd(2); m0123.AddToEnd(3); m0123.Canonicalize();
			c.AConnection = CFLConnection(atmp, m0123);
			NWAOBDDNodeHandle b0 = MkSwapNode(level-1, -1, controlled, 0);
			NWAOBDDNodeHandle b1 = MkSwapNode(level-1, -1, controlled, 1);
			NWAOBDDNodeHandle b2 = MkSwapNode(level-1, -1, controlled, 2);
			NWAOBDDNodeHandle b3 = MkSwapNode(level-1, -1, controlled, 3);
			ReturnMapHandle<int> m01, m10;
			m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
			m10.AddToEnd(1); m10.AddToEnd(0); m10.Canonicalize();
			c.numBConnections = 4;
			c.BConnection = new CFLConnection[4];
			c.BConnection[0] = CFLConnection(b0, m01);
			c.BConnection[1] = CFLConnection(b2, m10);
			c.BConnection[2] = CFLConnection(b1, m10);
			c.BConnection[3] = CFLConnection(b3, m10);
			c.numExits = 2;

			g = c.toNWA();
		}
		else if (level == 2 && controller == 0 && controlled == -1)
		{
			pseudoCFLOBDDInternal c(level);

			NWAOBDDNodeHandle atmp = MkSwapNode(level - 1, controller, -1, case_num);
			ReturnMapHandle<int> m0123;
			m0123.AddToEnd(0); m0123.AddToEnd(1); m0123.AddToEnd(2); m0123.AddToEnd(3); m0123.Canonicalize();
			c.AConnection = CFLConnection(atmp, m0123);
			ReturnMapHandle<int> m01, m21, m31, m41;
			m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
			m21.AddToEnd(2); m21.AddToEnd(1); m21.Canonicalize();
			m31.AddToEnd(3); m31.AddToEnd(1); m31.Canonicalize();
			m41.AddToEnd(4); m41.AddToEnd(1); m41.Canonicalize();
			c.numBConnections = 4;
			c.BConnection = new CFLConnection[4];
			NWAOBDDNodeHandle Id = MkIdNode(level-1);
			c.BConnection[0] = CFLConnection(Id, m01);
			c.BConnection[1] = CFLConnection(Id, m21);
			c.BConnection[2] = CFLConnection(Id, m31);
			c.BConnection[3] = CFLConnection(Id, m41);
			c.numExits = 5;	

			g = c.toNWA();
		}
		else if (level == 2 && controller == 1 && controlled == -1)
		{
			pseudoCFLOBDDInternal c(level);

			ReturnMapHandle<int> m01;
			m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
			NWAOBDDNodeHandle Id = MkIdNode(level-1);
			c.AConnection = CFLConnection(Id, m01);
			c.numBConnections = 2;
			NWAOBDDNodeHandle btmp = MkSwapNode(level-1, 0, -1, case_num);
			ReturnMapHandle<int> m0123, m4;
			m0123.AddToEnd(0); m0123.AddToEnd(1); m0123.AddToEnd(2); m0123.AddToEnd(3); m0123.Canonicalize();
			m4.AddToEnd(4); m4.Canonicalize();
			c.BConnection = new CFLConnection[2];
			c.BConnection[0] = CFLConnection(btmp, m0123);
			c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m4);
			c.numExits = 5;

			g = c.toNWA();
		}
		else 
		{
			pseudoCFLOBDDInternal c(level);
			unsigned int n = pow(2, level - 1);
			if (controller < n/2 && controlled < n/2 && controller >= 0 && controlled >= 0)
			{
				// Case 1: Both fall in A
				// std::cout << "Case1" << std::endl;
				NWAOBDDNodeHandle aTmp = MkSwapNode(level-1, controller, controlled, case_num);
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0);
				m01.AddToEnd(1);
				m01.Canonicalize();
				c.AConnection = CFLConnection(aTmp, m01);
				c.numBConnections = 2;
				c.BConnection = new CFLConnection[2];
				ReturnMapHandle<int> m1;
				m1.AddToEnd(1); m1.Canonicalize();
				NWAOBDDNodeHandle Id = MkIdNode(level - 1);
				c.BConnection[0] = CFLConnection(Id, m01);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.numExits = 2;
			}
			else if (controller >= n/2 && controlled >= n/2 && controller >= 0 && controlled >= 0)
			{
				// Case 2: Both fall in B region
				// std::cout << "Case2" << std::endl;
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0);
				m01.AddToEnd(1);
				m01.Canonicalize();
				NWAOBDDNodeHandle Id = MkIdNode(level - 1);
				c.AConnection = CFLConnection(Id, m01);
				c.numBConnections = 2;
				c.BConnection = new CFLConnection[2];
				NWAOBDDNodeHandle btmp = MkSwapNode(level-1, controller - n/2, controlled - n/2, case_num);
				c.BConnection[0] = CFLConnection(btmp, m01);
				ReturnMapHandle<int> m1;
				m1.AddToEnd(1); m1.Canonicalize();
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.numExits = 2;
			}
			else if (controller < n/2 && controlled >= n/2 && controller >= 0 && controlled >= 0)
			{
				// Case 3: controller in A and controlled in B
				// std::cout << "Case3" << std::endl;
				ReturnMapHandle<int> m01234;
				m01234.AddToEnd(0);
				m01234.AddToEnd(1);
				m01234.AddToEnd(2);
				m01234.AddToEnd(3);
				m01234.AddToEnd(4);
				m01234.Canonicalize();
				NWAOBDDNodeHandle atmp = MkSwapNode(level-1, controller, -1, case_num);
				c.AConnection = CFLConnection(atmp, m01234);
				c.numBConnections = 5;
				c.BConnection = new CFLConnection[5];
				NWAOBDDNodeHandle b0 = MkSwapNode(level-1, -1, controlled - n/2, 0);
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				c.BConnection[0] = CFLConnection(b0, m01);
				NWAOBDDNodeHandle b1 = MkSwapNode(level-1, -1, controlled - n/2, 2);
				NWAOBDDNodeHandle b2 = MkSwapNode(level-1, -1, controlled - n/2, 1);
				NWAOBDDNodeHandle b3 = MkSwapNode(level-1, -1, controlled - n/2, 3);
				ReturnMapHandle<int> m10, m1;
				m10.AddToEnd(1); m10.AddToEnd(0); m10.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				if (controller == n/2 - 1)
				{
					// std::cout << b1 << std::endl;
					// std::cout << b2 << std::endl;
					// std::cout << b3 << std::endl;
					c.BConnection[1] = CFLConnection(b1, m10);
					c.BConnection[2] = CFLConnection(b2, m10);
					c.BConnection[3] = CFLConnection(b3, m10);
					c.BConnection[4] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				}
				else
				{
					c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
					c.BConnection[2] = CFLConnection(b1, m10);
					c.BConnection[3] = CFLConnection(b2, m10);
					c.BConnection[4] = CFLConnection(b3, m10);
				}
				c.numExits = 2;
			}
			else if (controller < n/2 && controlled == -1 && controller >= 0)
			{
				// Case 4: controller in A and controlled == -1
				// std::cout << "Case4" << std::endl;
				ReturnMapHandle<int> m01234;
				m01234.AddToEnd(0);
				m01234.AddToEnd(1);
				m01234.AddToEnd(2);
				m01234.AddToEnd(3);
				m01234.AddToEnd(4);
				m01234.Canonicalize();
				NWAOBDDNodeHandle atmp = MkSwapNode(level-1, controller, -1, case_num);
				c.AConnection = CFLConnection(atmp, m01234);
				c.numBConnections = 5;
				c.BConnection = new CFLConnection[5];
				NWAOBDDNodeHandle Id = MkIdNode(level-1);
				ReturnMapHandle<int> m01, m21, m31, m41, m1;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				m21.AddToEnd(2); m21.AddToEnd(1); m21.Canonicalize();
				m31.AddToEnd(3); m31.AddToEnd(1); m31.Canonicalize();
				m41.AddToEnd(4); m41.AddToEnd(1); m41.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				if (controller == n/2 - 1)
				{
					c.BConnection[0] = CFLConnection(Id, m01);
					c.BConnection[1] = CFLConnection(Id, m21);
					c.BConnection[2] = CFLConnection(Id, m31);
					c.BConnection[3] = CFLConnection(Id, m41);
					c.BConnection[4] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				}
				else
				{
					c.BConnection[0] = CFLConnection(Id, m01);
					c.BConnection[2] = CFLConnection(Id, m21);
					c.BConnection[3] = CFLConnection(Id, m31);
					c.BConnection[4] = CFLConnection(Id, m41);
					c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);	
				}
				c.numExits = 5;
			}
			else if (controller >= n/2 && controlled == -1 && controller >= 0)
			{
				// Case 5: controller in B and controlled == -1
				// std::cout << "Case5" << std::endl;
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				NWAOBDDNodeHandle Id = MkIdNode(level-1);
				c.AConnection = CFLConnection(Id, m01);
				c.numBConnections = 2;
				NWAOBDDNodeHandle btmp = MkSwapNode(level-1, controller - n/2, -1, case_num);
				ReturnMapHandle<int> m0123, m1, m2, m4, m01234;
				m0123.AddToEnd(0); m0123.AddToEnd(1); m0123.AddToEnd(2); m0123.AddToEnd(3); m0123.Canonicalize();
				m01234.AddToEnd(0); m01234.AddToEnd(1); m01234.AddToEnd(2); m01234.AddToEnd(3); m01234.AddToEnd(4); m01234.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				m2.AddToEnd(2); m2.Canonicalize();
				m4.AddToEnd(4); m4.Canonicalize();
				c.BConnection = new CFLConnection[2];
				// if (controller == n - 1)
				// {
				// 	c.BConnection[0] = CFLConnection(btmp, m0123);
				// 	c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m4);
				// }
				// else
				// {
					c.BConnection[0] = CFLConnection(btmp, m01234);
					if (controller == n - 1)
						c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m4);
					else
						c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);

				// }
				c.numExits = 5;
			} 
			else if (controller == -1 && controlled < n/2 && controlled >= 0)
			{
				// Case 6: controller == -1 && controlled in A
				// std::cout << "Case6" << std::endl;
				NWAOBDDNodeHandle atmp = MkSwapNode(level-1, -1, controlled, case_num);
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				c.AConnection = CFLConnection(atmp, m01);
				c.numBConnections = 2;
				c.BConnection = new CFLConnection[2];
				NWAOBDDNodeHandle Id = MkIdNode(level - 1);
				ReturnMapHandle<int> m1, m0;
				m1.AddToEnd(1); m1.Canonicalize();
				m0.AddToEnd(0); m0.Canonicalize();
				if (case_num == 0)
				{
					c.BConnection[0] = CFLConnection(Id, m01);
					c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				}
				else{
					ReturnMapHandle<int> m10;
					m10.AddToEnd(1); m10.AddToEnd(0); m10.Canonicalize();
					c.BConnection[0] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m0);
					c.BConnection[1] = CFLConnection(Id, m10);
				}
				c.numExits = 2;
			}
			else if (controller == -1 && controlled >= n/2 && controlled >= 0)
			{
				// Case 7: controller == -1 && controlled in B
				// std::cout << "Case7" << std::endl;
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				NWAOBDDNodeHandle Id = MkIdNode(level - 1);
				c.AConnection = CFLConnection(Id, m01);
				c.numBConnections = 2;
				c.BConnection = new CFLConnection[2];
				ReturnMapHandle<int> m1, m0;
				m1.AddToEnd(1); m1.Canonicalize();
				m0.AddToEnd(0); m0.Canonicalize();
				NWAOBDDNodeHandle btmp = MkSwapNode(level-1, -1, controlled - n/2, case_num);
				if (case_num == 0)
				{
					c.BConnection[0] = CFLConnection(btmp, m01);
					c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				}
				else
				{
					c.BConnection[0] = CFLConnection(btmp, m01);
					c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m0);
				}
				c.numExits = 2;
			}
			g = c.toNWA();
		}
		g->InstallPathCounts();
		NWAOBDDNodeHandle gHandle = NWAOBDDNodeHandle(g);
		swap_hashMap.insert(std::make_pair(p, gHandle));
		return gHandle;
	}

	NWAOBDDNodeHandle MkiSwapNode(unsigned int level, long int controller, long int controlled, int case_num)
	{
		std::string p = std::to_string(level) + ";" + std::to_string(controller) + ";" + std::to_string(controlled) + ";" + std::to_string(case_num);
		if (iswap_hashMap.find(p) != iswap_hashMap.end()){
			return iswap_hashMap[p];
		}
		NWAOBDDInternalNode *g = new NWAOBDDInternalNode(level);
		if (level == 1)
		{
			pseudoCFLOBDDBase c;
			// std::cout << "CaseBase" << std::endl;
			if (case_num == -1)
			{
				assert(controlled == -1);
				c.numBConnections = 2;
				c.bconn[0] = intpair(0, 1);
				c.bconn[1] = intpair(2, 3);
				c.numExits = 4;
			}
			else if (case_num == 0)
			{
				// [[1 0] [0 0]]
				assert(controller == -1);
				c.numBConnections = 2;
				c.bconn[0] = intpair(0, 1);
				c.bconn[1] = intpair(1, 1);
				c.numExits = 2;
			}
			else if (case_num == 1)
			{
				// [[0 i] [0 0]]
				assert(controller == -1);
				c.numBConnections = 2;
				c.bconn[0] = intpair(0, 1);
				c.bconn[1] = intpair(0, 0);
				c.numExits = 2;	
			}
			else if (case_num == 2)
			{
				// [[0 0] [i 0]]
				assert(controller == -1);
				c.numBConnections = 2;
				c.bconn[0] = intpair(0, 0);
				c.bconn[1] = intpair(1, 0);
				c.numExits = 2;	
			}
			else if (case_num == 3)
			{
				// [[0 0] [0 1]]
				assert(controller == -1);
				c.numBConnections = 2;
				c.bconn[0] = intpair(0, 0);
				c.bconn[1] = intpair(0, 1);
				c.numExits = 2;	
			}
			g = c.toNWA();
		}
		else if (level == 2 && controller == 0 && controlled == 1)
		{
			pseudoCFLOBDDInternal c(level);

			NWAOBDDNodeHandle atmp = MkSwapNode(level - 1, controller, -1, case_num);
			ReturnMapHandle<int> m0123;
			m0123.AddToEnd(0); m0123.AddToEnd(1); m0123.AddToEnd(2); m0123.AddToEnd(3); m0123.Canonicalize();
			c.AConnection = CFLConnection(atmp, m0123);
			NWAOBDDNodeHandle b0 = MkSwapNode(level-1, -1, controlled, 0);
			NWAOBDDNodeHandle b1 = MkSwapNode(level-1, -1, controlled, 1);
			NWAOBDDNodeHandle b2 = MkSwapNode(level-1, -1, controlled, 2);
			NWAOBDDNodeHandle b3 = MkSwapNode(level-1, -1, controlled, 3);
			ReturnMapHandle<int> m01, m10, m12;
			m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
			m10.AddToEnd(1); m10.AddToEnd(0); m10.Canonicalize();
			m12.AddToEnd(1); m12.AddToEnd(2); m12.Canonicalize();
			c.numBConnections = 4;
			c.BConnection = new CFLConnection[4];
			c.BConnection[0] = CFLConnection(b0, m01);
			c.BConnection[1] = CFLConnection(b2, m12);
			c.BConnection[2] = CFLConnection(b1, m12);
			c.BConnection[3] = CFLConnection(b3, m10);
			c.numExits = 2;

			g = c.toNWA();
		}
		else if (level == 2 && controller == 0 && controlled == -1)
		{
			pseudoCFLOBDDInternal c(level);

			NWAOBDDNodeHandle atmp = MkSwapNode(level - 1, controller, -1, case_num);
			ReturnMapHandle<int> m0123;
			m0123.AddToEnd(0); m0123.AddToEnd(1); m0123.AddToEnd(2); m0123.AddToEnd(3); m0123.Canonicalize();
			c.AConnection = CFLConnection(atmp, m0123);
			ReturnMapHandle<int> m01, m21, m31, m41;
			m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
			m21.AddToEnd(2); m21.AddToEnd(1); m21.Canonicalize();
			m31.AddToEnd(3); m31.AddToEnd(1); m31.Canonicalize();
			m41.AddToEnd(4); m41.AddToEnd(1); m41.Canonicalize();
			c.numBConnections = 4;
			c.BConnection = new CFLConnection[4];
			NWAOBDDNodeHandle Id = MkIdNode(level-1);
			c.BConnection[0] = CFLConnection(Id, m01);
			c.BConnection[1] = CFLConnection(Id, m21);
			c.BConnection[2] = CFLConnection(Id, m31);
			c.BConnection[3] = CFLConnection(Id, m41);
			c.numExits = 5;	

			g = c.toNWA();
		}
		else if (level == 2 && controller == 1 && controlled == -1)
		{
			pseudoCFLOBDDInternal c(level);

			ReturnMapHandle<int> m01;
			m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
			NWAOBDDNodeHandle Id = MkIdNode(level-1);
			c.AConnection = CFLConnection(Id, m01);
			c.numBConnections = 2;
			NWAOBDDNodeHandle btmp = MkSwapNode(level-1, 0, -1, case_num);
			ReturnMapHandle<int> m0123, m4;
			m0123.AddToEnd(0); m0123.AddToEnd(1); m0123.AddToEnd(2); m0123.AddToEnd(3); m0123.Canonicalize();
			m4.AddToEnd(4); m4.Canonicalize();
			c.BConnection = new CFLConnection[2];
			c.BConnection[0] = CFLConnection(btmp, m0123);
			c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m4);
			c.numExits = 5;

			g = c.toNWA();
		}
		else 
		{
			pseudoCFLOBDDInternal c(level);

			unsigned int n = pow(2, level - 1);
			if (controller < n/2 && controlled < n/2 && controller >= 0 && controlled >= 0)
			{
				// Case 1: Both fall in A
				// std::cout << "Case1" << std::endl;
				NWAOBDDNodeHandle aTmp = MkSwapNode(level-1, controller, controlled, case_num);
				ReturnMapHandle<int> m012;
				m012.AddToEnd(0);
				m012.AddToEnd(1);
				m012.AddToEnd(2);
				m012.Canonicalize();
				c.AConnection = CFLConnection(aTmp, m012);
				c.numBConnections = 3;
				c.BConnection = new CFLConnection[2];
				ReturnMapHandle<int> m1;
				m1.AddToEnd(1); m1.Canonicalize();
				NWAOBDDNodeHandle Id = MkIdNode(level - 1);
				ReturnMapHandle<int> m01, m21;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				m21.AddToEnd(2); m21.AddToEnd(1); m21.Canonicalize();
				c.BConnection[0] = CFLConnection(Id, m01);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.BConnection[2] = CFLConnection(Id, m21);
				c.numExits = 3;
			}
			else if (controller >= n/2 && controlled >= n/2 && controller >= 0 && controlled >= 0)
			{
				// Case 2: Both fall in B region
				// std::cout << "Case2" << std::endl;
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0);
				m01.AddToEnd(1);
				m01.Canonicalize();
				NWAOBDDNodeHandle Id = MkIdNode(level - 1);
				c.AConnection = CFLConnection(Id, m01);
				c.numBConnections = 2;
				c.BConnection = new CFLConnection[2];
				NWAOBDDNodeHandle btmp = MkSwapNode(level-1, controller - n/2, controlled - n/2, case_num);
				ReturnMapHandle<int> m012;
				m012.AddToEnd(0); m012.AddToEnd(1); m012.AddToEnd(2); m012.Canonicalize();
				c.BConnection[0] = CFLConnection(btmp, m012);
				ReturnMapHandle<int> m1;
				m1.AddToEnd(1); m1.Canonicalize();
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.numExits = 3;
			}
			else if (controller < n/2 && controlled >= n/2 && controller >= 0 && controlled >= 0)
			{
				// Case 3: controller in A and controlled in B
				// std::cout << "Case3" << std::endl;
				ReturnMapHandle<int> m01234;
				m01234.AddToEnd(0);
				m01234.AddToEnd(1);
				m01234.AddToEnd(2);
				m01234.AddToEnd(3);
				m01234.AddToEnd(4);
				m01234.Canonicalize();
				NWAOBDDNodeHandle atmp = MkSwapNode(level-1, controller, -1, case_num);
				c.AConnection = CFLConnection(atmp, m01234);
				c.numBConnections = 5;
				c.BConnection = new CFLConnection[5];
				NWAOBDDNodeHandle b0 = MkSwapNode(level-1, -1, controlled - n/2, 0);
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				c.BConnection[0] = CFLConnection(b0, m01);
				NWAOBDDNodeHandle b1 = MkSwapNode(level-1, -1, controlled - n/2, 2);
				NWAOBDDNodeHandle b2 = MkSwapNode(level-1, -1, controlled - n/2, 1);
				NWAOBDDNodeHandle b3 = MkSwapNode(level-1, -1, controlled - n/2, 3);
				ReturnMapHandle<int> m10, m1, m12;
				m10.AddToEnd(1); m10.AddToEnd(0); m10.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				m12.AddToEnd(1); m12.AddToEnd(2); m12.Canonicalize();
				if (controller == n/2 - 1)
				{
					// std::cout << b1 << std::endl;
					// std::cout << b2 << std::endl;
					// std::cout << b3 << std::endl;
					c.BConnection[1] = CFLConnection(b1, m12);
					c.BConnection[2] = CFLConnection(b2, m12);
					c.BConnection[3] = CFLConnection(b3, m10);
					c.BConnection[4] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				}
				else
				{
					c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
					c.BConnection[2] = CFLConnection(b1, m12);
					c.BConnection[3] = CFLConnection(b2, m12);
					c.BConnection[4] = CFLConnection(b3, m10);
				}
				c.numExits = 3;
			}
			else if (controller < n/2 && controlled == -1 && controller >= 0)
			{
				// Case 4: controller in A and controlled == -1
				// std::cout << "Case4" << std::endl;
				ReturnMapHandle<int> m01234;
				m01234.AddToEnd(0);
				m01234.AddToEnd(1);
				m01234.AddToEnd(2);
				m01234.AddToEnd(3);
				m01234.AddToEnd(4);
				m01234.Canonicalize();
				NWAOBDDNodeHandle atmp = MkSwapNode(level-1, controller, -1, case_num);
				c.AConnection = CFLConnection(atmp, m01234);
				c.numBConnections = 5;
				c.BConnection = new CFLConnection[5];
				NWAOBDDNodeHandle Id = MkIdNode(level-1);
				ReturnMapHandle<int> m01, m21, m31, m41, m1;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				m21.AddToEnd(2); m21.AddToEnd(1); m21.Canonicalize();
				m31.AddToEnd(3); m31.AddToEnd(1); m31.Canonicalize();
				m41.AddToEnd(4); m41.AddToEnd(1); m41.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				if (controller == n/2 - 1)
				{
					c.BConnection[0] = CFLConnection(Id, m01);
					c.BConnection[1] = CFLConnection(Id, m21);
					c.BConnection[2] = CFLConnection(Id, m31);
					c.BConnection[3] = CFLConnection(Id, m41);
					c.BConnection[4] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				}
				else
				{
					c.BConnection[0] = CFLConnection(Id, m01);
					c.BConnection[2] = CFLConnection(Id, m21);
					c.BConnection[3] = CFLConnection(Id, m31);
					c.BConnection[4] = CFLConnection(Id, m41);
					c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);	
				}
				c.numExits = 5;
			}
			else if (controller >= n/2 && controlled == -1 && controller >= 0)
			{
				// Case 5: controller in B and controlled == -1
				// std::cout << "Case5" << std::endl;
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				NWAOBDDNodeHandle Id = MkIdNode(level-1);
				c.AConnection = CFLConnection(Id, m01);
				c.numBConnections = 2;
				NWAOBDDNodeHandle btmp = MkSwapNode(level-1, controller - n/2, -1, case_num);
				ReturnMapHandle<int> m0123, m1, m2, m4, m01234;
				m0123.AddToEnd(0); m0123.AddToEnd(1); m0123.AddToEnd(2); m0123.AddToEnd(3); m0123.Canonicalize();
				m01234.AddToEnd(0); m01234.AddToEnd(1); m01234.AddToEnd(2); m01234.AddToEnd(3); m01234.AddToEnd(4); m01234.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				m2.AddToEnd(2); m2.Canonicalize();
				m4.AddToEnd(4); m4.Canonicalize();
				c.BConnection = new CFLConnection[2];
				// if (controller == n - 1)
				// {
				// 	c.BConnection[0] = CFLConnection(btmp, m0123);
				// 	c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m4);
				// }
				// else
				// {
					c.BConnection[0] = CFLConnection(btmp, m01234);
					if (controller == n - 1)
						c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m4);
					else
						c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);

				// }
				c.numExits = 5;
			} 
			else if (controller == -1 && controlled < n/2 && controlled >= 0)
			{
				// Case 6: controller == -1 && controlled in A
				// std::cout << "Case6" << std::endl;
				NWAOBDDNodeHandle atmp = MkSwapNode(level-1, -1, controlled, case_num);
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				c.AConnection = CFLConnection(atmp, m01);
				c.numBConnections = 2;
				c.BConnection = new CFLConnection[2];
				NWAOBDDNodeHandle Id = MkIdNode(level - 1);
				ReturnMapHandle<int> m1, m0;
				m1.AddToEnd(1); m1.Canonicalize();
				m0.AddToEnd(0); m0.Canonicalize();
				if (case_num == 0)
				{
					c.BConnection[0] = CFLConnection(Id, m01);
					c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				}
				else{
					ReturnMapHandle<int> m10;
					m10.AddToEnd(1); m10.AddToEnd(0); m10.Canonicalize();
					c.BConnection[0] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m0);
					c.BConnection[1] = CFLConnection(Id, m10);
				}
				c.numExits = 2;
			}
			else if (controller == -1 && controlled >= n/2 && controlled >= 0)
			{
				// Case 7: controller == -1 && controlled in B
				// std::cout << "Case7" << std::endl;
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				NWAOBDDNodeHandle Id = MkIdNode(level - 1);
				c.AConnection = CFLConnection(Id, m01);
				c.numBConnections = 2;
				c.BConnection = new CFLConnection[2];
				ReturnMapHandle<int> m1, m0;
				m1.AddToEnd(1); m1.Canonicalize();
				m0.AddToEnd(0); m0.Canonicalize();
				NWAOBDDNodeHandle btmp = MkSwapNode(level-1, -1, controlled - n/2, case_num);
				if (case_num == 0)
				{
					c.BConnection[0] = CFLConnection(btmp, m01);
					c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				}
				else
				{
					c.BConnection[0] = CFLConnection(btmp, m01);
					c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m0);
				}
				c.numExits = 2;
			}

			g = c.toNWA();
		}
		g->InstallPathCounts();
		NWAOBDDNodeHandle gHandle = NWAOBDDNodeHandle(g);
		iswap_hashMap.insert(std::make_pair(p, gHandle));
		return gHandle;
	}

	NWAOBDDNodeHandle MkCSwap2Node(unsigned int level, long int controller, long int index1, long int index2, int case_num) {
		std::string p = std::to_string(level) + ";" + std::to_string(controller) + ";" + std::to_string(index1) + ";" + std::to_string(index2) + ";" + std::to_string(case_num);
		if (cswap_hashMap.find(p) != cswap_hashMap.end()){
			return cswap_hashMap[p];
		}

		NWAOBDDInternalNode *g = new NWAOBDDInternalNode(level);
		if (level == 1)
		{
			pseudoCFLOBDDBase c;
			// std::cout << "CaseBase" << std::endl;
			if (controller == 0 && index1 == -1 && index2 == -1)
			{
				c.numBConnections = 2;
				c.bconn[0] = intpair(0, 1);
				c.bconn[1] = intpair(1, 2);
				c.numExits = 3;
			}
			g = c.toNWA();
		}
		else 
		{
			pseudoCFLOBDDInternal c(level);

			unsigned int n = pow(2, level - 1);
			if (controller < n/2 && index1 < n/2 && index2 < n/2 && controller >= 0 && index1 >= 0 && index2 >= 0)
			{
				// Case 1: All fall in A
				// std::cout << "Case1" << std::endl;
				NWAOBDDNodeHandle aTmp = MkCSwap2Node(level-1, controller, index1, index2, case_num);
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0);
				m01.AddToEnd(1);
				m01.Canonicalize();
				c.AConnection = CFLConnection(aTmp, m01);
				c.numBConnections = 2;
				c.BConnection = new CFLConnection[2];
				ReturnMapHandle<int> m1;
				m1.AddToEnd(1); m1.Canonicalize();
				NWAOBDDNodeHandle Id = MkIdNode(level - 1);
				c.BConnection[0] = CFLConnection(Id, m01);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.numExits = 2;
			}
			else if (controller >= n/2 && index1 >= n/2 && index2 >= n/2 && controller >= 0 && index1 >= 0 && index2 >= 0)
			{
				// Case 2: All fall in B region
				// std::cout << "Case2" << std::endl;
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0);
				m01.AddToEnd(1);
				m01.Canonicalize();
				NWAOBDDNodeHandle Id = MkIdNode(level - 1);
				c.AConnection = CFLConnection(Id, m01);
				c.numBConnections = 2;
				c.BConnection = new CFLConnection[2];
				NWAOBDDNodeHandle btmp = MkCSwap2Node(level-1, controller - n/2, index1 - n/2, index2 - n/2, case_num);
				c.BConnection[0] = CFLConnection(btmp, m01);
				ReturnMapHandle<int> m1;
				m1.AddToEnd(1); m1.Canonicalize();
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.numExits = 2;
			}
			else if (controller < n/2 && index1 >= n/2 && index2 >= n/2 && controller >= 0 && index1 >= 0 && index2 >= 0)
			{
				// Case 3: controller in A and index1 and index2 in B
				// std::cout << "Case3" << std::endl;
				ReturnMapHandle<int> m012;
				m012.AddToEnd(0); m012.AddToEnd(1); m012.AddToEnd(2); m012.Canonicalize();
				NWAOBDDNodeHandle atmp = MkCSwap2Node(level-1, controller, -1, -1, case_num);
				c.AConnection = CFLConnection(atmp, m012);
				c.numBConnections = 3;
				c.BConnection = new CFLConnection[3];
				NWAOBDDNodeHandle bb = MkSwapNode(level-1, index1 - n/2, index2 - n/2, -1);
				ReturnMapHandle<int> m01, m1;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				auto Id = MkIdNode(level - 1);
				c.BConnection[0] = CFLConnection(Id, m01);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.BConnection[2] = CFLConnection(bb, m01);
				c.numExits = 2;
			}
			else if (controller < n/2 && index1 == -1 && index2 == -1 && controller >= 0)
			{
				// Case 4: controller in A and index1 == index2 == -1
				// std::cout << "Case4" << std::endl;
				ReturnMapHandle<int> m012;
				m012.AddToEnd(0);
				m012.AddToEnd(1);
				m012.AddToEnd(2);
				m012.Canonicalize();
				NWAOBDDNodeHandle atmp = MkCSwap2Node(level-1, controller, -1, -1, case_num);
				c.AConnection = CFLConnection(atmp, m012);
				c.numBConnections = 3;
				c.BConnection = new CFLConnection[3];
				NWAOBDDNodeHandle Id = MkIdNode(level-1);
				ReturnMapHandle<int> m01, m21, m1;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				m21.AddToEnd(2); m21.AddToEnd(1); m21.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				c.BConnection[0] = CFLConnection(Id, m01);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.BConnection[2] = CFLConnection(Id, m21);
				c.numExits = 3;
			}
			else if (controller >= n/2 && index1 == -1 && index2 == -1 && controller >= 0)
			{
				// Case 5: controller in B and index1 == index2 == -1
				// std::cout << "Case5" << std::endl;
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				NWAOBDDNodeHandle Id = MkIdNode(level-1);
				c.AConnection = CFLConnection(Id, m01);
				c.numBConnections = 2;
				NWAOBDDNodeHandle btmp = MkCSwap2Node(level-1, controller - n/2, -1, -1, case_num);
				ReturnMapHandle<int> m012, m1;
				m012.AddToEnd(0); m012.AddToEnd(1); m012.AddToEnd(2); m012.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				c.BConnection = new CFLConnection[2];
				c.BConnection[0] = CFLConnection(btmp, m012);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m1);
				c.numExits = 3;
			} 
			else if (controller == -1 && index2 < n/2 && index2 >= 0 && index1 == -1)
			{
				// Case 6: controller == -1 && index2 in A && index1 == -1
				// std::cout << "Case6" << std::endl;
				NWAOBDDNodeHandle atmp = MkSwapNode(level-1, -1, index2, case_num);
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				c.AConnection = CFLConnection(atmp, m01);
				c.numBConnections = 2;
				c.BConnection = new CFLConnection[2];
				NWAOBDDNodeHandle Id = MkIdNode(level - 1);
				ReturnMapHandle<int> m1, m0;
				m1.AddToEnd(1); m1.Canonicalize();
				m0.AddToEnd(0); m0.Canonicalize();
				if (case_num == 0)
				{
					c.BConnection[0] = CFLConnection(Id, m01);
					c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				}
				else{
					ReturnMapHandle<int> m10;
					m10.AddToEnd(1); m10.AddToEnd(0); m10.Canonicalize();
					c.BConnection[0] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m0);
					c.BConnection[1] = CFLConnection(Id, m10);
				}
				c.numExits = 2;
			}
			else if (controller == -1 && index2 >= n/2 && index2 >= 0 && index1 == -1)
			{
				// Case 7: controller == -1 && index2 in B && index1 == -1
				// std::cout << "Case7" << std::endl;
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				NWAOBDDNodeHandle Id = MkIdNode(level - 1);
				c.AConnection = CFLConnection(Id, m01);
				c.numBConnections = 2;
				c.BConnection = new CFLConnection[2];
				ReturnMapHandle<int> m1, m0;
				m1.AddToEnd(1); m1.Canonicalize();
				m0.AddToEnd(0); m0.Canonicalize();
				NWAOBDDNodeHandle btmp = MkSwapNode(level-1, -1, index2 - n/2, case_num);
				if (case_num == 0)
				{
					c.BConnection[0] = CFLConnection(btmp, m01);
					c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				}
				else
				{
					c.BConnection[0] = CFLConnection(btmp, m01);
					c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m0);
				}
				c.numExits = 2;
			}
			else if (controller < n/2 && index1 < n/2 && index2 >= n/2 && controller >= 0 && index1 >= 0 && index2 >= 0)
			{
				// Case 8: CR in A, index1 in A, index2 in B
				ReturnMapHandle<int> m012345;
				m012345.AddToEnd(0);
				m012345.AddToEnd(1);
				m012345.AddToEnd(2);
				m012345.AddToEnd(3);
				m012345.AddToEnd(4);
				m012345.AddToEnd(5);
				m012345.Canonicalize();

				auto aa = MkCSwap2Node(level - 1, controller, index1, -1, case_num);
				c.AConnection = CFLConnection(aa, m012345);
				c.numBConnections = 6;
				c.BConnection = new CFLConnection[6];
				auto Id = MkIdNode(level - 1);
				ReturnMapHandle<int> m01, m1, m10;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				m10.AddToEnd(1); m10.AddToEnd(0); m10.Canonicalize();
				c.BConnection[0] = CFLConnection(Id, m01);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				auto b0 = MkSwapNode(level-1, -1, index2 - n/2, 0);
				auto b1 = MkSwapNode(level-1, -1, index2 - n/2, 2);
				auto b2 = MkSwapNode(level-1, -1, index2 - n/2, 1);
				auto b3 = MkSwapNode(level-1, -1, index2 - n/2, 3);
				c.BConnection[2] = CFLConnection(b0, m01);
				c.BConnection[3] = CFLConnection(b1, m10);
				c.BConnection[4] = CFLConnection(b2, m10);
				c.BConnection[5] = CFLConnection(b3, m10);
				c.numExits = 2;
			}
			else if (controller < n/2 && index1 < n/2 && index2 == -1 && controller >= 0 && index1 >= 0)
			{
				// Case 9: CR and index1 in A, index2 == -1
				ReturnMapHandle<int> m012345;
				m012345.AddToEnd(0);
				m012345.AddToEnd(1);
				m012345.AddToEnd(2);
				m012345.AddToEnd(3);
				m012345.AddToEnd(4);
				m012345.AddToEnd(5);
				m012345.Canonicalize();

				auto aa = MkCSwap2Node(level - 1, controller, index1, -1, case_num);
				c.AConnection = CFLConnection(aa, m012345);
				c.numBConnections = 6;
				c.BConnection = new CFLConnection[6];
				auto Id = MkIdNode(level - 1);
				ReturnMapHandle<int> m01, m1, m21, m31, m41, m51;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				m21.AddToEnd(2); m21.AddToEnd(1); m21.Canonicalize();
				m31.AddToEnd(2); m31.AddToEnd(1); m31.Canonicalize();
				m41.AddToEnd(2); m41.AddToEnd(1); m41.Canonicalize();
				m51.AddToEnd(2); m51.AddToEnd(1); m51.Canonicalize();
				c.BConnection[0] = CFLConnection(Id, m01);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.BConnection[2] = CFLConnection(Id, m21);
				c.BConnection[3] = CFLConnection(Id, m31);
				c.BConnection[4] = CFLConnection(Id, m41);
				c.BConnection[5] = CFLConnection(Id, m51);
				c.numExits = 6;
			}
			else if (controller >= n/2 && index1 >= n/2 && index2 == -1 && controller >= 0 && index1 >= 0)
			{
				// Case 9: CR and index1 in B, index2 == -1
				auto Id = MkIdNode(level - 1);
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				ReturnMapHandle<int> m012345;
				m012345.AddToEnd(0);
				m012345.AddToEnd(1);
				m012345.AddToEnd(2);
				m012345.AddToEnd(3);
				m012345.AddToEnd(4);
				m012345.AddToEnd(5);
				m012345.Canonicalize();

				ReturnMapHandle<int> m1;
				m1.AddToEnd(1); m1.Canonicalize();

				c.AConnection = CFLConnection(Id, m01);
				c.numBConnections = 2;
				c.BConnection = new CFLConnection[2];
				auto aa = MkCSwap2Node(level - 1, controller - n/2, index1 - n/2, -1, case_num);
				c.BConnection[0] = CFLConnection(aa, m012345);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.numExits = 2;
			}
			else if (controller < n/2 && index1 >= n/2 && index2 == -1 && controller >= 0)
			{
				// Case 10: CR in A, index1 in B and index2 == -1
				ReturnMapHandle<int> m012;
				m012.AddToEnd(0); m012.AddToEnd(1); m012.AddToEnd(2); m012.Canonicalize();
				auto aa = MkCSwap2Node(level - 1, controller, -1, -1, case_num);
				c.AConnection = CFLConnection(aa, m012);
				c.numBConnections = 3;
				c.BConnection = new CFLConnection[3];
				auto Id = MkIdNode(level - 1);
				ReturnMapHandle<int> m01, m1;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				c.BConnection[0] = CFLConnection(Id, m01);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				auto bb = MkSwapNode(level-1, index1 - n/2, -1, case_num);
				ReturnMapHandle<int> m21345;
				m21345.AddToEnd(2);
				m21345.AddToEnd(1);
				m21345.AddToEnd(3);
				m21345.AddToEnd(4);
				m21345.AddToEnd(5);
				m21345.Canonicalize();
				c.BConnection[2] = CFLConnection(bb, m21345);
				c.numExits = 6;
			}

			g = c.toNWA();
			
		}
		g->InstallPathCounts();
		NWAOBDDNodeHandle gHandle = NWAOBDDNodeHandle(g);
		swap_hashMap.insert(std::make_pair(p, gHandle));
		return gHandle;
	}

	NWAOBDDNodeHandle MkCPNode(unsigned int level, long int controller, long int controlled)
	{
		std::string p = std::to_string(level) + ";" + std::to_string(controller) + ";" + std::to_string(controlled);
		if (cp_hashMap.find(p) != cp_hashMap.end()){
			return cp_hashMap[p];
		}
		NWAOBDDInternalNode *g;
		if (level == 1)
		{
			// std::cout << "CaseBase" << std::endl;
			pseudoCFLOBDDBase c;
			
			c.numBConnections = 2;
			c.bconn[0] = intpair(0, 1);
			c.bconn[1] = intpair(1, 2);
			c.numExits = 3;

			g = c.toNWA();
		}
		else 
		{
			pseudoCFLOBDDInternal c(level);

			unsigned int n = pow(2, level - 1);
			if (controller < n/2 && controlled < n/2 && controller >= 0 && controlled >= 0)
			{
				// Case 1: Both fall in A
				// std::cout << "Case1" << std::endl;
				NWAOBDDNodeHandle aTmp = MkCPNode(level-1, controller, controlled);
				ReturnMapHandle<int> m012;
				m012.AddToEnd(0);
				m012.AddToEnd(1);
				m012.AddToEnd(2);
				m012.Canonicalize();
				c.AConnection = CFLConnection(aTmp, m012);
				c.numBConnections = 3;
				c.BConnection = new CFLConnection[3];
				ReturnMapHandle<int> m01, m1, m21;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				m21.AddToEnd(2); m21.AddToEnd(1); m21.Canonicalize();
				NWAOBDDNodeHandle Id = MkIdNode(level - 1);
				c.BConnection[0] = CFLConnection(Id, m01);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.BConnection[2] = CFLConnection(Id, m21);
				c.numExits = 3;
			}
			else if (controller >= n/2 && controlled >= n/2 && controller >= 0 && controlled >= 0)
			{
				// Case 2: Both fall in B region
				// std::cout << "Case2" << std::endl;
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0);
				m01.AddToEnd(1);
				m01.Canonicalize();
				NWAOBDDNodeHandle Id = MkIdNode(level - 1);
				c.AConnection = CFLConnection(Id, m01);
				c.numBConnections = 2;
				c.BConnection = new CFLConnection[2];
				ReturnMapHandle<int> m012;
				m012.AddToEnd(0); m012.AddToEnd(1); m012.AddToEnd(2); m012.Canonicalize();
				NWAOBDDNodeHandle btmp = MkCPNode(level-1, controller - n/2, controlled - n/2);
				c.BConnection[0] = CFLConnection(btmp, m012);
				ReturnMapHandle<int> m1;
				m1.AddToEnd(1); m1.Canonicalize();
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.numExits = 3;
			}
			else if (controller < n/2 && controlled >= n/2 && controller >= 0 && controlled >= 0)
			{
				// Case 3: controller in A and controlled in B
				// std::cout << "Case3" << std::endl;
				ReturnMapHandle<int> m012;
				m012.AddToEnd(0);
				m012.AddToEnd(1);
				m012.AddToEnd(2);
				m012.Canonicalize();
				NWAOBDDNodeHandle atmp = MkCPNode(level-1, controller, -1);
				c.AConnection = CFLConnection(atmp, m012);
				c.numBConnections = 3;
				c.BConnection = new CFLConnection[3];
				ReturnMapHandle<int> m01, m1;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				NWAOBDDNodeHandle Id = MkIdNode(level-1);
				c.BConnection[0] = CFLConnection(Id, m01);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				NWAOBDDNodeHandle btmp = MkCPNode(level-1, -1, controlled - n/2);
				c.BConnection[2] = CFLConnection(btmp, m012);
				c.numExits = 3;
			}
			else if (controller < n/2 && controlled == -1 && controller >= 0)
			{
				// Case 4: controller in A and controlled == -1
				// std::cout << "Case4" << std::endl;
				ReturnMapHandle<int> m012;
				m012.AddToEnd(0);
				m012.AddToEnd(1);
				m012.AddToEnd(2);
				m012.Canonicalize();
				NWAOBDDNodeHandle atmp = MkCPNode(level-1, controller, -1);
				c.AConnection = CFLConnection(atmp, m012);
				c.numBConnections = 3;
				c.BConnection = new CFLConnection[3];
				NWAOBDDNodeHandle Id = MkIdNode(level-1);
				ReturnMapHandle<int> m01, m21, m1;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				m21.AddToEnd(2); m21.AddToEnd(1); m21.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				c.BConnection[0] = CFLConnection(Id, m01);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.BConnection[2] = CFLConnection(Id, m21);
				c.numExits = 3;
			}
			else if (controller >= n/2 && controlled == -1 && controller >= 0)
			{
				// Case 5: controller in B and controlled == -1
				// std::cout << "Case5" << std::endl;
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				NWAOBDDNodeHandle Id = MkIdNode(level-1);
				c.AConnection = CFLConnection(Id, m01);
				c.numBConnections = 2;
				NWAOBDDNodeHandle btmp = MkCPNode(level-1, controller - n/2, -1);
				ReturnMapHandle<int> m012, m1;
				m012.AddToEnd(0); m012.AddToEnd(1); m012.AddToEnd(2); m012.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				c.BConnection = new CFLConnection[2];
				c.BConnection[0] = CFLConnection(btmp, m012);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.numExits = 3;
			} 
			else if (controller == -1 && controlled < n/2 && controlled >= 0)
			{
				// Case 6: controller == -1 && controlled in A
				// std::cout << "Case6" << std::endl;
				NWAOBDDNodeHandle atmp = MkCPNode(level-1, -1, controlled);
				ReturnMapHandle<int> m012;
				m012.AddToEnd(0); m012.AddToEnd(1); m012.AddToEnd(2); m012.Canonicalize();
				c.AConnection = CFLConnection(atmp, m012);
				c.numBConnections = 3;
				c.BConnection = new CFLConnection[3];
				NWAOBDDNodeHandle Id = MkIdNode(level - 1);
				ReturnMapHandle<int> m01, m21, m1;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				m21.AddToEnd(2); m21.AddToEnd(1); m21.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				c.BConnection[0] = CFLConnection(Id, m01);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.BConnection[2] = CFLConnection(Id, m21);
				c.numExits = 3;
			}
			else if (controller == -1 && controlled >= n/2 && controlled >= 0)
			{
				// Case 7: controller == -1 && controlled in B
				// std::cout << "Case7" << std::endl;
				ReturnMapHandle<int> m01;
				m01.AddToEnd(0); m01.AddToEnd(1); m01.Canonicalize();
				NWAOBDDNodeHandle Id = MkIdNode(level - 1);
				c.AConnection = CFLConnection(Id, m01);
				c.numBConnections = 2;
				c.BConnection = new CFLConnection[2];
				ReturnMapHandle<int> m012, m1;
				m012.AddToEnd(0); m012.AddToEnd(1); m012.AddToEnd(2); m012.Canonicalize();
				m1.AddToEnd(1); m1.Canonicalize();
				NWAOBDDNodeHandle btmp = MkCPNode(level-1, -1, controlled - n/2);
				c.BConnection[0] = CFLConnection(btmp, m012);
				c.BConnection[1] = CFLConnection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m1);
				c.numExits = 3;
			}
			
			g = c.toNWA();
		}
		g->InstallPathCounts();
		NWAOBDDNodeHandle gHandle = NWAOBDDNodeHandle(g);
		cp_hashMap.insert(std::make_pair(p, gHandle));
		return gHandle;
	}

}