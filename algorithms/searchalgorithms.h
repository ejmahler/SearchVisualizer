#ifndef SEARCHALGORITHMS_H
#define SEARCHALGORITHMS_H

#include <functional>
#include <algorithm>

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <queue>

class SearchAlgorithms
{
public:

	template<class State>
	static std::vector<State> aStar(
		const std::vector<State> &startStates,

		//predicate that returns true if the given state is a goal state
		std::function<bool(const State &currentState)> goalFunction,

		//function to call on each expanded state as it is processed
		std::function<void(const State &currentState, const State &parentState)> stateFunction,

		//function uses to get the neighbors of a given state, and the cost to move to each state
		std::function<std::vector<std::pair<State, float>>(const State &currentState)> neighborFunction,

		//function to compute the heuristic for the given state
		std::function<float(const State &currentState)> heuristicFunction
		);

private:
	SearchAlgorithms() = default;
};

template<class State>
std::vector<State> SearchAlgorithms::aStar(
	const std::vector<State> &startStates,

	//predicate that returns true if the given state is a goal state
	std::function<bool(const State &currentState)> goalFunction,

	//function to call on each expanded state as it is processed
	std::function<void(const State &currentState, const State &parentState)> stateFunction,

	//function uses to get the neighbors of a given state, and the cost to move to each state
	std::function<std::vector<std::pair<State, float>>(const State &currentState)> neighborFunction,

	//function to compute the heuristic for the given state
	std::function<float(const State &currentState)> heuristicFunction
	)
{
	struct SearchItem {
		State state;
		State parent;
		float currentCost;
		float estimatedTotalCost;

		SearchItem(){}

		SearchItem(const QPoint &state, const QPoint &parent, float currentCost, float estimatedTotalCost)
			:state(state), parent(parent), currentCost(currentCost), estimatedTotalCost(estimatedTotalCost)
		{}


		bool operator<(const SearchItem &other) const
		{
			if (estimatedTotalCost != other.estimatedTotalCost)
				return estimatedTotalCost > other.estimatedTotalCost;
			else
			{
				float heuristic = estimatedTotalCost - currentCost;
				float otherHeuristic = other.estimatedTotalCost - other.currentCost;

				return heuristic > otherHeuristic;
			}
		}

	};

	//build the priority queue from the initial set of items
	std::priority_queue<SearchItem> openSet;
	for (const auto& initialState : startStates)
	{
		openSet.emplace(initialState, initialState, 0.0f, heuristicFunction(initialState));
	}

	std::unordered_map<State, State> closedSet;

	State goalState;
	bool foundGoal = false;

	//loop until we've gone though every node
	while (!openSet.empty())
	{
		SearchItem currentState = openSet.top();
		openSet.pop();

		//check if we've already hit this state
		if (!closedSet.count(currentState.state))
		{
			closedSet[currentState.state] = currentState.parent;

			//call the state function to process this state
			stateFunction(currentState.state, currentState.parent);

			//if we've reached a goal state, end the loop
			if (goalFunction(currentState.state))
			{
				foundGoal = true;
				goalState = currentState.state;
				break;
			}

			//loop through the neighbors of this state
			auto neighbors = neighborFunction(currentState.state);
			for (const auto& neighbor : neighbors)
			{
				//"neighbor" is a 2-tuple, first item is the state and the second is the cost to move to that state

				//if we haven't already viewed this neighbor
				if (!closedSet.count(neighbor.first))
				{
					float totalCost = neighbor.second + currentState.currentCost;
					float totalEstimatedCost = totalCost + heuristicFunction(neighbor.first);

					openSet.emplace(neighbor.first, currentState.state, totalCost, totalEstimatedCost);
				}
			}
		}
	}

	//we left the loop, see if we hit a goal state. if we didn't, the result vector will be empty
	std::vector<State> result;
	if (foundGoal)
	{
		State currentItem = goalState;
		State currentParent = closedSet.at(goalState);

		result.push_back(currentItem);

		while (currentItem != currentParent)
		{
			currentItem = currentParent;
			currentParent = closedSet.at(currentItem);

			result.push_back(currentItem);
		}
	}
	std::reverse(result.begin(), result.end());

	return result;
}


#endif // SEARCHALGORITHMS_H
