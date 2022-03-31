#pragma once

#include <list>
#include <memory>
#include <stdexcept>
#include <functional>

#include "IEventHandler.hpp"
#include "SnakeInterface.hpp"

class Event;
class IPort;

namespace Snake
{
	struct Segment
	{
        int x;
        int y;
	};

	struct Body : std::list<Segment>
	{
		bool isSegmentAtPosition(int x, int y) const;
		void removeSegment(IPort& displayPort);
		void addHeadSegment(Segment const& newHead, IPort& displayPort);
		Segment calculateNewHead(Direction& currentDirection) const;
		void removeSegmentIfNotScored(Segment const& newHead, std::pair<int,int> foodPosition,
				IPort& scorePort, IPort& foodPort, IPort& displayPort);
		void updateIfSuccessfullMove(Segment const& newHead, bool isOutsideMap,
				std::pair<int,int>& foodPosition,
				IPort& scorePort, IPort& displayPort, IPort& foodPort);
	};

struct ConfigurationError : std::logic_error
{
    ConfigurationError();
};

struct UnexpectedEventException : std::runtime_error
{
    UnexpectedEventException();
};

class Controller : public IEventHandler
{
public:
    Controller(IPort& p_displayPort, IPort& p_foodPort, IPort& p_scorePort, std::string const& p_config);

    Controller(Controller const& p_rhs) = delete;
    Controller& operator=(Controller const& p_rhs) = delete;

    void receive(std::unique_ptr<Event> e) override;

private:
    IPort& m_displayPort;
    IPort& m_foodPort;
    IPort& m_scorePort;

    std::pair<int, int> m_mapDimension;
    std::pair<int, int> m_foodPosition;


    Body m_body;
    Direction m_currentDirection;

    void handleTimeoutInd();
    void handleDirectionInd(std::unique_ptr<Event>);
    void handleFoodInd(std::unique_ptr<Event>);
    void handleFoodResp(std::unique_ptr<Event>);
    void handlePauseInd(std::unique_ptr<Event>);

    // void removeTailSegmentIfNotScored(Segment const& newHead);

    bool isPositionOutsideMap(int x, int y) const;

    void updateFoodPosition(int x, int y, std::function<void()> clearPolicy);
    void sendClearOldFood();
    void sendPlaceNewFood(int x, int y);

    bool m_paused;
};

} // namespace Snake
