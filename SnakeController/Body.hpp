#pragma once
	struct Segment
	{
        int x;
        int y;
	};

	struct Body : std::list<Segment>
	{
		Body(IPort& p_displayPort, IPort& p_foodPort, IPort& p_scorePort);
		bool isSegmentAtPosition(int x, int y) const;
		void removeSegment();
		void addHeadSegment(Segment const& newHead);
		Segment calculateNewHead(Direction& currentDirection) const;
		void removeSegmentIfNotScored(Segment const& newHead, std::pair<int,int> foodPosition);
		void updateIfSuccessfullMove(Segment const& newHead, bool isOutsideMap,
				std::pair<int,int>& foodPosition);
		IPort& m_displayPort;
		IPort& m_foodPort;
		IPort& m_scorePort;
	};


