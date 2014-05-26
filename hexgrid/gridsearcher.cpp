#include "gridsearcher.h"

#include <algorithm>
#include <unordered_set>

#include "hexgrid/hexgrid.h"
#include "utils/channel.h"
#include "algorithms/searchalgorithms.h"

//define a hash function for QPoint
namespace std {
	template <> struct hash < QPoint >
	{
		size_t operator()(const QPoint &point) const
		{
			int hashSize = sizeof(size_t);

			size_t a = std::hash<int>()(point.x());
			size_t b = std::hash<int>()(point.y());

			size_t c = (b << (hashSize / 2)) | (b >> (hashSize / 2));

			return a ^ c;
		}
	};
}


GridSearcher::GridSearcher(HexGrid &grid) :
	grid(grid)
{
}

void GridSearcher::search(std::shared_ptr<Channel<GridSearchEvent>> outputChannel)
{

	std::vector<QPoint> startStates;
	std::unordered_set<QPoint> goalStates;

	//find all end states and start states in the grid
	for (const auto& cell : grid.getCells())
	{
		const GridEntry& entry = grid.getEntry(cell);

		if (entry.type == GridEntry::Start)
		{
			startStates.push_back(cell);
		}
		else if (entry.type == GridEntry::End)
		{
			goalStates.insert(cell);
		}
	}

	//define a function that returns true if the given state is a goal state
	auto goalFunction = [&goalStates](const QPoint &currentState)
	{
		return bool(goalStates.count(currentState));
	};

	//define a function that "processes" the given state when it's reached
	auto stateFunction = [&outputChannel](const QPoint &currentState, const QPoint &parentState)
	{
        Q_UNUSED(parentState)
		outputChannel->push(GridSearchEvent(GridSearchEvent::EXPAND, currentState));
	};

	//define a function that returns the neighbors and associated costs for the given state
	auto neighborFunction = [this](const QPoint &currentState)
	{
		QVector<QPoint> neighbors = grid.getNeighbors(currentState);
		std::vector<std::pair<QPoint, float>> result;
		for (const QPoint &n : neighbors)
		{
			if (grid.getEntry(n).type != GridEntry::Wall)
			{
				result.emplace_back(n, 1.0f);
			}
		}

		return result;
	};

	//define a function that returns the heuristic for the given state
	auto heuristicFunction = [&](const QPoint &currentState)
	{
		float minDistance = grid.manhattanDistance(currentState, *(goalStates.begin()));

		for (const QPoint &p : goalStates)
		{
			float d = grid.manhattanDistance(currentState, p);
			minDistance = qMin(minDistance, d);
		}

		return minDistance;
	};

	//perform the search
	std::vector<QPoint> result = SearchAlgorithms::aStar<QPoint>(startStates, goalFunction, stateFunction, neighborFunction, heuristicFunction);

	//put out a search event for each item in the final route, in reversed order, to simulate backtracing the result
	std::reverse(result.begin(), result.end());
	for (const QPoint& item : result)
	{
		outputChannel->push(GridSearchEvent(GridSearchEvent::BACKTRACE, item));
	}

	//close the output channel to wrap things up
	outputChannel->closeBack();
}
