#include <fstream>
#include <thread>
#include <mutex>
#include <cmath>
#include <cstdlib>
#include "mcts.h"

const char* LOG_FILE_FORMAT = "MCTS%d.log";
const char* LOG_FILE_FULL = "MCTS_FULL.log";
const float Cp = 1.0f;
const float SEARCH_TIME_MIN = 0.05f;
const float SEARCH_TIME_MAX = 0.2f;
const int	EXPAND_THRESHOLD = 1;
const bool	ENABLE_MULTI_THREAD = true;

const int	FAST_STOP_ESTIMATE_COUNT = 4;
const int	FAST_STOP_STEPS_MAX = 400;
const int	FAST_STOP_STEPS_MIN = 100;

const bool	ENABLE_TRY_MORE_NODE = false;
const int	TRY_MORE_NODE_THRESHOLD = 1000;

TreeNode::TreeNode(TreeNode *p)
{
	visit = 0;
	value = 0;
	winRate = 0;
	expandFactor = 0;
	validActionCount = 0;
	gridLevel = 0;
	game = NULL;
	parent = p;
}

FILE *fp;

MCTS::MCTS(int mode)
{
	this->mode = mode;

	root = NULL;

	// clear log file
	for (int i = 0; i < 20; ++i)
	{
		char logFile[20];
		sprintf_s(logFile, 20, LOG_FILE_FORMAT, i + 1);

		fopen_s(&fp, logFile, "w");
		fclose(fp);
	}
}

MCTS::~MCTS()
{
	ClearPool();
}

mutex mtx;

void MCTS::SearchThread(int id, int seed, MCTS *mcts, clock_t startTime, float searchTime)
{
	srand(seed); // need to call srand for each thread
	float elapsedTime = 0;

	while (1)
	{
		mtx.lock();
		TreeNode *node = mcts->TreePolicy(mcts->root);
		mtx.unlock();

		float value = mcts->DefaultPolicy(node, id);

		mtx.lock();
		mcts->UpdateValue(node, value);
		mtx.unlock();

		elapsedTime = float(clock() - startTime) / 1000;
		if (elapsedTime > searchTime)
		{
			mtx.lock();
			TreeNode *mostVisit = *max_element(mcts->root->children.begin(), mcts->root->children.end(), [](const TreeNode *a, const TreeNode *b)
			{
				return a->visit < b->visit;
			});

			TreeNode *bestScore = mcts->BestChild(mcts->root, 0);
			mtx.unlock();
			
			break;
			//if (mostVisit == bestScore)
			//	break;
		}
	}
}

int MCTS::Search(Game *state)
{
	fastStopSteps = 0;
	fastStopCount = 0;

	root = NewTreeNode(NULL);
	*(root->game) = *((GameBase*)state);
	root->game->GetValidActions(root->validActions, root->validActionCount);

	float boardRatio = clamp(6 - root->game->validGridCount, 1, 5) / 5.f;
	float turnRatio = clamp((root->game->turn - 400.f) / 800.f, 0.f, 1.f);
	float timeRatio = boardRatio * turnRatio;
	float searchTime = SEARCH_TIME_MAX * timeRatio + SEARCH_TIME_MIN * (1 - timeRatio);

	clock_t startTime = clock();

	thread threads[THREAD_NUM_MAX];
	int thread_num = ENABLE_MULTI_THREAD ? thread::hardware_concurrency() : 1;

	for (int i = 0; i < thread_num; ++i)
		threads[i] = thread(SearchThread, i, rand(), this, startTime, searchTime);

	for (int i = 0; i < thread_num; ++i)
		threads[i].join();

	TreeNode *best = BestChild(root, 0);
	int move = best->game->lastMove;

	maxDepth = 0;
	PrintTree(root);
	PrintFullTree(root);
	printf("plan: %.2f, time: %.2f, iteration: %d, depth: %d, win: %.2f%% (%d/%d)\n", searchTime, float(clock() - startTime) / 1000, root->visit, maxDepth, best->value * 100 / best->visit, (int)best->value, best->visit);
	printf("fast stop count: %d, average stop steps: %d\n", fastStopCount, fastStopSteps / (fastStopCount + 1));

	ClearNodes(root);

	return move;
}

TreeNode* MCTS::TreePolicy(TreeNode *node)
{
	while (!node->game->IsGameFinish())
	{
		if (node->visit < EXPAND_THRESHOLD)
			return node;

		if (PreExpandTree(node))
			return ExpandTree(node);
		else
			node = BestChild(node, Cp);
	}
	return node;
}

bool MCTS::PreExpandTree(TreeNode *node)
{
	if (node->validActionCount > 0)
	{
		int id = rand() % node->validActionCount;
		swap(node->validActions[id], node->validActions[node->validActionCount - 1]);
	}
	else
	{
		// try grids with lower priority after certain visits
		/*if (ENABLE_TRY_MORE_NODE && node->gridLevel == 0 && node->visit > TRY_MORE_NODE_THRESHOLD * node->children.size())
		{
			if (node->game->UpdateValidGridsExtra())
			{
				node->gridLevel++;
				node->validGrids = node->game->validGrids;
				node->validGridCount = node->game->validGridCount;

				int id = rand() % node->validGridCount;
				swap(node->validGrids[id], node->validGrids[node->validGridCount - 1]);
			}
		}*/
	}

	return node->validActionCount > 0;
}

TreeNode* MCTS::ExpandTree(TreeNode *node)
{
	int move = node->validActions[node->validActionCount - 1];
	--(node->validActionCount);

	TreeNode *newNode = NewTreeNode(node);
	node->children.push_back(newNode);
	*(newNode->game) = *(node->game);
	newNode->game->Move(move);
	newNode->game->GetValidActions(newNode->validActions, newNode->validActionCount);

	return newNode;
}

TreeNode* MCTS::BestChild(TreeNode *node, float c)
{
	TreeNode *result = NULL;
	float bestScore = -1;
	float expandFactorParent_c = sqrtf(logf(node->visit)) * c;

	for (auto child : node->children)
	{
		float score = CalcScoreFast(child, expandFactorParent_c);
		if (score > bestScore)
		{
			bestScore = score;
			result = child;
		}
	}
	return result;
}

float MCTS::CalcScore(const TreeNode *node, float c, float logParentVisit)
{
	float winRate = node->value / node->visit;
	float expandFactor = c * sqrtf(logParentVisit / node->visit);

	if (node->game->GetSide() == root->game->GetSide()) // win rate of opponent
		winRate = 1 - winRate;

	return winRate + expandFactor;
}

float MCTS::CalcScoreFast(const TreeNode *node, float expandFactorParent_c)
{
	return node->winRate + node->expandFactor * expandFactorParent_c;
}

float MCTS::DefaultPolicy(TreeNode *node, int id)
{
	gameCache[id] = *(node->game);

	float bestValue = 0;
	int estimateCount = 0;

	int turnCount = 0;
	float timeRatio = clamp((gameCache[id].turn - 200.f) / 1000.f, 0.f, 1.f);
	int fastStopStep = FAST_STOP_STEPS_MIN * timeRatio + FAST_STOP_STEPS_MAX * (1 - timeRatio);

	while (!gameCache[id].IsGameFinish())
	{
		int move = gameCache[id].GetNextMove();
		gameCache[id].Move(move);

		if (++turnCount > fastStopStep)
		{
			float value = gameCache[id].CalcFastStopScore();
			bestValue = max(bestValue, value);

			if (++estimateCount > FAST_STOP_ESTIMATE_COUNT)
			{
				fastStopCount++;
				fastStopSteps += gameCache[id].turn - node->game->turn;
				return bestValue;
			}
		}
	}
	float ratio = (float)turnCount / fastStopStep;
	return gameCache[id].CalcFinishScore(ratio);
}

void MCTS::UpdateValue(TreeNode *node, float value)
{
	while (node != NULL)
	{
		node->visit++;
		node->value += value;

		node->expandFactor = sqrtf(1.f / node->visit);
		node->winRate = node->value / node->visit;

		if (node->game->GetSide() == root->game->GetSide()) // win rate of opponent
			node->winRate = 1 - node->winRate;

		node = node->parent;
	}
}

void MCTS::ClearNodes(TreeNode *node)
{
	if (node != NULL)
	{
		for (auto child : node->children)
		{
			ClearNodes(child);
		}

		RecycleTreeNode(node);
	}
}

TreeNode* MCTS::NewTreeNode(TreeNode *parent)
{
	if (pool.empty())
	{
		TreeNode *node = new TreeNode(parent);
		node->game = new GameBase();
		return node;
	}

	TreeNode *node = pool.back();
	node->parent = parent;
	pool.pop_back();

	return node;
}

void MCTS::RecycleTreeNode(TreeNode *node)
{
	node->parent = NULL;
	node->visit = 0;
	node->value = 0;
	node->winRate = 0;
	node->expandFactor = 0;
	node->validActionCount = 0;
	node->gridLevel = 0;
	node->children.clear();

	pool.push_back(node);
}

void MCTS::ClearPool()
{
	for (auto node : pool)
	{
		delete node->game;
		delete node;
	}
}

void MCTS::PrintTree(TreeNode *node, int level)
{
	if (level == 1)
	{
		int logId = node->game->turn / 100 + 1;
		char logFile[20];
		sprintf_s(logFile, 20, LOG_FILE_FORMAT, logId);

		freopen_s(&fp, logFile, "a+", stdout);
		node->game->board.Print();
		fclose(stdout);
		freopen_s(&fp, "CON", "w", stdout);

		fopen_s(&fp, logFile, "a+");
		fprintf(fp, "===============================PrintTree=============================\n");
		fprintf(fp, "visit: %d, value: %.1f, children: %d\n", node->visit, node->value, node->children.size());
	}

	if (level > maxDepth)
		maxDepth = level;

	node->children.sort([](const TreeNode *a, const TreeNode *b)
	{
		return a->visit > b->visit;
	});

	int i = 1;
	for (auto it = node->children.begin(); it != node->children.end(); ++it)
	{
		fprintf(fp, "%d", level);
		for (int j = 0; j < level; ++j)
			fprintf(fp, "   ");

		float expandFactorParent_c = sqrtf(logf(node->visit)) * Cp;
		fprintf(fp, "visit: %d, value: %.1f, raw_score: %.6f, score: %.6f, children: %d, move: %s\n", (*it)->visit, (*it)->value, CalcScoreFast(*it, 0), CalcScoreFast(*it, expandFactorParent_c), (*it)->children.size(), (*it)->game->LastAction2Str().c_str());
		PrintTree(*it, level + 1);

		if (++i > 4)
			break;
	}

	if (level == 1)
	{
		fprintf(fp,"================================TreeEnd============================\n\n");
		fclose(fp);
	}
}

void MCTS::PrintFullTree(TreeNode *node, int level)
{
	if (level == 1)
	{
		fopen_s(&fp, LOG_FILE_FULL, "w");
		fprintf(fp, "===============================PrintFullTree=============================\n");
		fprintf(fp, "visit: %d, value: %.1f, children: %d\n", node->visit, node->value, node->children.size());
	}

	node->children.sort([](const TreeNode *a, const TreeNode *b)
	{
		return a->visit > b->visit;
	});

	int i = 1;
	for (auto it = node->children.begin(); it != node->children.end(); ++it)
	{
		fprintf(fp, "%d", level);
		for (int j = 0; j < level; ++j)
			fprintf(fp, "   ");

		float expandFactorParent_c = sqrtf(logf(node->visit)) * Cp;
		fprintf(fp, "visit: %d, value: %.1f, raw_score: %.6f, score: %.6f, children: %d, move: %s\n", (*it)->visit, (*it)->value, CalcScoreFast(*it, 0), CalcScoreFast(*it, expandFactorParent_c), (*it)->children.size(), (*it)->game->LastAction2Str().c_str());
		PrintFullTree(*it, level + 1);
	}

	if (level == 1)
	{
		fprintf(fp, "================================TreeEnd============================\n\n");
		fclose(fp);
	}
}
