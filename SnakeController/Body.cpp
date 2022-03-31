#include "Body.hpp"

namespace Snake {

namespace {
bool isHorizontal(Direction direction);
bool isVertical(Direction direction);
bool isPositive(Direction direction);
bool perpendicular(Direction dir1, Direction dir2);
}

Body::Body(IPort& p_displayPort, IPort& p_foodPort, IPort& p_scorePort) :
	m_displayPort(p_displayPort),
	m_foodPort(p_foodPort),
	m_scorePort(p_scorePort)
{
}

bool Body::isSegmentAtPosition(int x, int y) const
{
	return end() !=  std::find_if(cbegin(), cend(),
			[x, y](auto const& segment){ return segment.x == x and segment.y == y; });
}

void Body::removeSegment()
{
	auto tail = back();

	DisplayInd l_evt;
	l_evt.x = tail.x;
	l_evt.y = tail.y;
	l_evt.value = Cell_FREE;
	m_displayPort.send(std::make_unique<EventT<DisplayInd>>(l_evt));

	pop_back();
}

void Body::addHeadSegment(Segment const& newHead)
{
    push_front(newHead);

    DisplayInd placeNewHead;
    placeNewHead.x = newHead.x;
    placeNewHead.y = newHead.y;
    placeNewHead.value = Cell_SNAKE;

    m_displayPort.send(std::make_unique<EventT<DisplayInd>>(placeNewHead));
}

Segment Body::calculateNewHead(Direction& currentDirection) const
{
    Segment const& currentHead = front();

    Segment newHead;
    newHead.x = currentHead.x + (isHorizontal(currentDirection) ? isPositive(currentDirection) ? 1 : -1 : 0);
    newHead.y = currentHead.y + (isVertical(currentDirection) ? isPositive(currentDirection) ? 1 : -1 : 0);

    return newHead;
}

void Body::removeSegmentIfNotScored(Segment const& newHead, std::pair<int,int> foodPosition)
{
    if (std::make_pair(newHead.x, newHead.y) == foodPosition) {
        m_scorePort.send(std::make_unique<EventT<ScoreInd>>());
        m_foodPort.send(std::make_unique<EventT<FoodReq>>());
    } else {
        removeSegment();
    }
}

void Body::updateIfSuccessfullMove(Segment const& newHead, bool isOutsideMap,
		std::pair<int,int>& foodPosition)
{
    if (isSegmentAtPosition(newHead.x, newHead.y)
			or /*isPositionOutsideMap(newHead.x, newHead.y)*/ isOutsideMap)
	{
        m_scorePort.send(std::make_unique<EventT<LooseInd>>());
    } else {
        addHeadSegment(newHead);
        removeSegmentIfNotScored(newHead, foodPosition);
    }
}
}
