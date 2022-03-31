#include "SnakeController.hpp"

#include <algorithm>
#include <sstream>

#include "EventT.hpp"
#include "IPort.hpp"

namespace Snake
{

namespace
{
bool isHorizontal(Direction direction)
{
    return Direction_LEFT == direction or Direction_RIGHT == direction;
}

bool isVertical(Direction direction)
{
    return Direction_UP == direction or Direction_DOWN == direction;
}

bool isPositive(Direction direction)
{
    return (isVertical(direction) and Direction_DOWN == direction)
        or (isHorizontal(direction) and Direction_RIGHT == direction);
}

bool perpendicular(Direction dir1, Direction dir2)
{
    return isHorizontal(dir1) == isVertical(dir2);
}
} // namespace

bool Body::isSegmentAtPosition(int x, int y) const
{
	return end() !=  std::find_if(cbegin(), cend(),
			[x, y](auto const& segment){ return segment.x == x and segment.y == y; });
}

void Body::removeSegment(IPort& displayPort)
{
	auto tail = back();

	DisplayInd l_evt;
	l_evt.x = tail.x;
	l_evt.y = tail.y;
	l_evt.value = Cell_FREE;
	displayPort.send(std::make_unique<EventT<DisplayInd>>(l_evt));

	pop_back();
}

void Body::addHeadSegment(Segment const& newHead, IPort& displayPort)
{
    push_front(newHead);

    DisplayInd placeNewHead;
    placeNewHead.x = newHead.x;
    placeNewHead.y = newHead.y;
    placeNewHead.value = Cell_SNAKE;

    displayPort.send(std::make_unique<EventT<DisplayInd>>(placeNewHead));
}

Segment Body::calculateNewHead(Direction& currentDirection) const
{
    Segment const& currentHead = front();

    Segment newHead;
    newHead.x = currentHead.x + (isHorizontal(currentDirection) ? isPositive(currentDirection) ? 1 : -1 : 0);
    newHead.y = currentHead.y + (isVertical(currentDirection) ? isPositive(currentDirection) ? 1 : -1 : 0);

    return newHead;
}

/*
void Body::removeSegmentIfNotScored(Segment const& newHead, std::pair<int,int> foodPosition, IPort& scorePort, IPort& foodPort, IPort& displayPort)
{
    if (std::make_pair(newHead.x, newHead.y) == foodPosition) {
        scorePort.send(std::make_unique<EventT<ScoreInd>>());
        foodPort.send(std::make_unique<EventT<FoodReq>>());
    } else {
        removeSegment(displayPort);
    }
}
*/

void Body::updateIfSuccessfullMove(Segment const& newHead, IPort& scorePort, IPort& displayPort, bool isOutsideMap, IPort& foodPort, std::pair<int,int>& foodPosition)
{
    if (isSegmentAtPosition(newHead.x, newHead.y) or
			isOutsideMap // isPositionOutsideMap(newHead.x, newHead.y)
			) {
        scorePort.send(std::make_unique<EventT<LooseInd>>());
    } else {
        addHeadSegment(newHead, displayPort);



        // removeSegmentIfNotScored(newHead);
    if (std::make_pair(newHead.x, newHead.y) == foodPosition) {
        scorePort.send(std::make_unique<EventT<ScoreInd>>());
        foodPort.send(std::make_unique<EventT<FoodReq>>());
    } else {
        removeSegment(displayPort);
    }



    }
}

ConfigurationError::ConfigurationError()
    : std::logic_error("Bad configuration of Snake::Controller.")
{}

UnexpectedEventException::UnexpectedEventException()
    : std::runtime_error("Unexpected event received!")
{}

Controller::Controller(IPort& p_displayPort, IPort& p_foodPort, IPort& p_scorePort, std::string const& p_config)
    : m_displayPort(p_displayPort),
      m_foodPort(p_foodPort),
      m_scorePort(p_scorePort),
      m_paused(false)
{
    std::istringstream istr(p_config);
    char w, f, s, d;

    int width, height, length;
    int foodX, foodY;
    istr >> w >> width >> height >> f >> foodX >> foodY >> s;

    if (w == 'W' and f == 'F' and s == 'S') {
        m_mapDimension = std::make_pair(width, height);
        m_foodPosition = std::make_pair(foodX, foodY);

        istr >> d;
        switch (d) {
            case 'U':
                m_currentDirection = Direction_UP;
                break;
            case 'D':
                m_currentDirection = Direction_DOWN;
                break;
            case 'L':
                m_currentDirection = Direction_LEFT;
                break;
            case 'R':
                m_currentDirection = Direction_RIGHT;
                break;
            default:
                throw ConfigurationError();
        }
        istr >> length;

        while (length--) {
            Segment seg;
            istr >> seg.x >> seg.y;
            m_body.push_back(seg);
        }
    } else {
        throw ConfigurationError();
    }
}

bool Controller::isPositionOutsideMap(int x, int y) const
{
    return x < 0 or y < 0 or x >= m_mapDimension.first or y >= m_mapDimension.second;
}

void Controller::sendPlaceNewFood(int x, int y)
{
    m_foodPosition = std::make_pair(x, y);

    DisplayInd placeNewFood;
    placeNewFood.x = x;
    placeNewFood.y = y;
    placeNewFood.value = Cell_FOOD;

    m_displayPort.send(std::make_unique<EventT<DisplayInd>>(placeNewFood));
}

void Controller::sendClearOldFood()
{
    DisplayInd clearOldFood;
    clearOldFood.x = m_foodPosition.first;
    clearOldFood.y = m_foodPosition.second;
    clearOldFood.value = Cell_FREE;

    m_displayPort.send(std::make_unique<EventT<DisplayInd>>(clearOldFood));
}

void Controller::handleTimeoutInd()
{
	auto newHead = m_body.calculateNewHead(m_currentDirection);
    m_body.updateIfSuccessfullMove(newHead, m_scorePort, m_displayPort,
			isPositionOutsideMap(newHead.x, newHead.y), m_foodPort, m_foodPosition
			);
}

void Controller::handleDirectionInd(std::unique_ptr<Event> e)
{
    auto direction = payload<DirectionInd>(*e).direction;

    if (perpendicular(m_currentDirection, direction)) {
        m_currentDirection = direction;
    }
}

void Controller::updateFoodPosition(int x, int y, std::function<void()> clearPolicy)
{
    if (m_body.isSegmentAtPosition(x, y) || isPositionOutsideMap(x,y)) {
        m_foodPort.send(std::make_unique<EventT<FoodReq>>());
        return;
    }

    clearPolicy();
    sendPlaceNewFood(x, y);
}

void Controller::handleFoodInd(std::unique_ptr<Event> e)
{
    auto receivedFood = payload<FoodInd>(*e);

    updateFoodPosition(receivedFood.x, receivedFood.y, std::bind(&Controller::sendClearOldFood, this));
}

void Controller::handleFoodResp(std::unique_ptr<Event> e)
{
    auto requestedFood = payload<FoodResp>(*e);

    updateFoodPosition(requestedFood.x, requestedFood.y, []{});
}

void Controller::handlePauseInd(std::unique_ptr<Event> e)
{
    m_paused = not m_paused;
}

void Controller::receive(std::unique_ptr<Event> e)
{
    switch (e->getMessageId()) {
        case TimeoutInd::MESSAGE_ID:
            if (!m_paused) {
                return handleTimeoutInd();
            }
            return;
        case DirectionInd::MESSAGE_ID:
            if (!m_paused) {
                return handleDirectionInd(std::move(e));
            }
            return;
        case FoodInd::MESSAGE_ID:
            return handleFoodInd(std::move(e));
        case FoodResp::MESSAGE_ID:
            return handleFoodResp(std::move(e));
        case PauseInd::MESSAGE_ID:
            return handlePauseInd(std::move(e));
        default:
            throw UnexpectedEventException();
    }
}

} // namespace Snake
