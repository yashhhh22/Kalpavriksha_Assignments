#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "Players_data.h"

#define MAX_NAME_LENGTH 50
#define MAX_PLAYERS_PER_TEAM 50

typedef struct
{
    int id;
    char fullName[MAX_NAME_LENGTH];
    char teamName[MAX_NAME_LENGTH];
    char roleName[MAX_NAME_LENGTH];
    int runs;
    float battingAvg;
    float strikeRate;
    int wickets;
    float economy;
    float performanceScore;
} PlayerRecord;

typedef struct
{
    int id;
    char teamName[MAX_NAME_LENGTH];
    int totalPlayers;
    float averageBattingStrikeRate;
    int playerIndexes[MAX_PLAYERS_PER_TEAM];
    int batsmanIndexes[MAX_PLAYERS_PER_TEAM];
    int bowlerIndexes[MAX_PLAYERS_PER_TEAM];
    int allrounderIndexes[MAX_PLAYERS_PER_TEAM];
    int batsmanCount;
    int bowlerCount;
    int allrounderCount;
} TeamRecord;

typedef struct
{
    int playerIndex;
    int teamIndex;
    int indexInRoleList;
} TeamPlayerHeapNode;

void populatePlayersFromData(PlayerRecord playerRecords[], int *loadedPlayerCount);
void buildTeamsFromPlayers(TeamRecord teamRecords[], int *loadedTeamCount, PlayerRecord playerRecords[], int loadedPlayerCount);
int findTeamIndexById(TeamRecord teamRecords[], int teamId, int loadedTeamCount);
void sortRoleIndexArrayByPerformance(PlayerRecord players[], int indexArray[], int startIndex, int endIndex);
void mergeRoleIndexSegments(PlayerRecord players[], int indexArray[], int leftStart, int leftMid, int rightEnd);
void sortTeamsByAverageStrikeRate(TeamRecord teams[], int startIndex, int endIndex);
void mergeTeamSegmentsByAverage(TeamRecord teams[], int startIndex, int middleIndex, int endIndex);
void showPlayersForTeamId(TeamRecord teams[], PlayerRecord players[], int teamId , int loadedTeamCount);
void showTeamsOrderedByStrikeRate(TeamRecord teams[], int loadedTeamCount);
void showTopKPlayersForRoleInTeam(TeamRecord teams[], PlayerRecord players[], int loadedTeamCount);
void swapHeapNodes(TeamPlayerHeapNode *firstNode, TeamPlayerHeapNode *secondNode);
void heapSiftDown(TeamPlayerHeapNode heap[], int heapSize, int currentIndex, PlayerRecord players[]);
void heapInsertNode(TeamPlayerHeapNode heap[], int *heapSize, TeamPlayerHeapNode nodeToInsert, PlayerRecord players[]);
TeamPlayerHeapNode heapPopMaxNode(TeamPlayerHeapNode heap[], int *heapSize, PlayerRecord players[]);
void showAllPlayersByRoleAcrossTeams(TeamRecord teams[], PlayerRecord players[], int roleChoice, int loadedTeamCount);
int isValidIntegerString(char *inputString);
int isValidFloatString(char *inputString);
int readValidatedInteger();
float readValidatedFloat();
void addNewPlayerToTeam(TeamRecord teams[], PlayerRecord players[], int *loadedPlayerCount, int loadedTeamCount);

int isValidIntegerString(char *inputString)
{
    int position = 0;
    if (inputString[0] == '\0') 
    {
        return 0;
    }
    if (inputString[0] == '+' || inputString[0] == '-') 
    {
        position = 1;
    }
    for (; inputString[position] != '\0'; position++)
    {
        if (inputString[position] < '0' || inputString[position] > '9') 
        {
            return 0;
        }
    }
    return 1;
}

int readValidatedInteger()
{
    char buffer[MAX_NAME_LENGTH];
    while (1)
    {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
        {
            clearerr(stdin);
            printf("Invalid input! Enter a valid integer: ");
            continue;
        }
        int position = 0;
        while (buffer[position] != '\n' && buffer[position] != '\0') 
        {
            position++;
        }
        buffer[position] = '\0';
        if (isValidIntegerString(buffer)) 
        {
            return atoi(buffer);
        }
        printf("Invalid input! Enter a valid integer: ");
    }
}

int isValidFloatString(char *inputString)
{
    int position = 0;
    int decimalPointFound = 0;
    if (inputString[0] == '\0') 
    {
        return 0;
    }
    if (inputString[0] == '+' || inputString[0] == '-') 
    {
        position = 1;
    }
    int digitFound = 0;
    for (; inputString[position] != '\0'; position++)
    {
        if (inputString[position] == '.')
        {
            if (decimalPointFound) 
            {
                return 0;
            }
            decimalPointFound = 1;
            continue;
        }
        if (inputString[position] < '0' || inputString[position] > '9') 
        {
            return 0;
        }
        digitFound = 1;
    }
    return digitFound;
}

float readValidatedFloat()
{
    char buffer[MAX_NAME_LENGTH];
    while (1)
    {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
        {
            clearerr(stdin);
            printf("Invalid input! Enter a valid float: ");
            continue;
        }
        int position = 0;
        while (buffer[position] != '\n' && buffer[position] != '\0') 
        {
            position++;
        }
        buffer[position] = '\0';
        if (isValidFloatString(buffer)) 
        {
            return (float)atof(buffer);
        }
        printf("Invalid input! Enter a valid float: ");
    }
}

void populatePlayersFromData(PlayerRecord playerRecords[], int *loadedPlayerCount)
{
    *loadedPlayerCount = 0;
    for (int dataIndex = 0; dataIndex < playerCount; dataIndex++)
    {
        playerRecords[dataIndex].id = players[dataIndex].id;
        strncpy(playerRecords[dataIndex].fullName, players[dataIndex].name, MAX_NAME_LENGTH - 1);
        playerRecords[dataIndex].fullName[MAX_NAME_LENGTH - 1] = '\0';
        strncpy(playerRecords[dataIndex].teamName, players[dataIndex].team, MAX_NAME_LENGTH - 1);
        playerRecords[dataIndex].teamName[MAX_NAME_LENGTH - 1] = '\0';
        strncpy(playerRecords[dataIndex].roleName, players[dataIndex].role, MAX_NAME_LENGTH - 1);
        playerRecords[dataIndex].roleName[MAX_NAME_LENGTH - 1] = '\0';
        playerRecords[dataIndex].runs = players[dataIndex].totalRuns;
        playerRecords[dataIndex].battingAvg = players[dataIndex].battingAverage;
        playerRecords[dataIndex].strikeRate = players[dataIndex].strikeRate;
        playerRecords[dataIndex].wickets = players[dataIndex].wickets;
        playerRecords[dataIndex].economy = players[dataIndex].economyRate;
        if (strcmp(players[dataIndex].role, "Batsman") == 0)
        {
            playerRecords[dataIndex].performanceScore = (players[dataIndex].battingAverage * players[dataIndex].strikeRate) / 100.0f;
        }
        else if (strcmp(players[dataIndex].role, "Bowler") == 0)
        {
            playerRecords[dataIndex].performanceScore = (players[dataIndex].wickets * 2) + (100 - players[dataIndex].economyRate);
        }
        else
        {
            playerRecords[dataIndex].performanceScore = ((players[dataIndex].battingAverage * players[dataIndex].strikeRate) / 100.0f) + (players[dataIndex].wickets * 2);
        }
        (*loadedPlayerCount)++;
    }
}

void mergeRoleIndexSegments(PlayerRecord players[], int indexArray[], int leftStart, int leftMid, int rightEnd)
{
    int leftSize = leftMid - leftStart + 1;
    int rightSize = rightEnd - leftMid;

    if (leftSize <= 0 && rightSize <= 0) 
    {
        return;
    }

    int *leftSegment = NULL;
    int *rightSegment = NULL;

    if (leftSize > 0)
    {
        leftSegment = (int *)malloc(leftSize * sizeof(int));
        if (leftSegment == NULL)
        {
            printf("Memory allocation failed in mergeRoleIndexSegments (leftSegment).\n");
            return;
        }
        for (int i = 0; i < leftSize; ++i)
        {
            leftSegment[i] = indexArray[leftStart + i];
        }    
    }

    if (rightSize > 0)
    {
        rightSegment = (int *)malloc(rightSize * sizeof(int));
        if (rightSegment == NULL)
        {
            printf("Memory allocation failed in mergeRoleIndexSegments (rightSegment).\n");
            free(leftSegment);
            return;
        }
        for (int size = 0; size < rightSize; ++size)
        {
            rightSegment[size] = indexArray[leftMid + 1 + size];
        }
    }

    int leftPosition = 0;
    int rightPosition = 0;
    int writePosition = leftStart;

    while ((leftSegment != NULL && leftPosition < leftSize) && (rightSegment != NULL && rightPosition < rightSize))
    {
        if (players[leftSegment[leftPosition]].performanceScore >= players[rightSegment[rightPosition]].performanceScore)
        {
            indexArray[writePosition++] = leftSegment[leftPosition++];
        }
        else
        {
            indexArray[writePosition++] = rightSegment[rightPosition++];
        }
    }

    while (leftSegment != NULL && leftPosition < leftSize)
    {
        indexArray[writePosition++] = leftSegment[leftPosition++];
    }

    while (rightSegment != NULL && rightPosition < rightSize)
    {
        indexArray[writePosition++] = rightSegment[rightPosition++];
    }

    free(leftSegment);
    free(rightSegment);
}

void mergeTeamSegmentsByAverage(TeamRecord teams[], int startIndex, int middleIndex, int endIndex)
{
    int leftSize = middleIndex - startIndex + 1;
    int rightSize = endIndex - middleIndex;

    if (leftSize <= 0 && rightSize <= 0) 
    {
        return;
    }

    TeamRecord *leftArray = NULL;
    TeamRecord *rightArray = NULL;

    if (leftSize > 0)
    {
        leftArray = (TeamRecord *)malloc(leftSize * sizeof(TeamRecord));
        if (leftArray == NULL)
        {
            printf("Memory allocation failed in mergeTeamSegmentsByAverage (leftArray).\n");
            return;
        }
        for (int index = 0; index < leftSize; ++index)
        {
            leftArray[index] = teams[startIndex + index];
        }
    }

    if (rightSize > 0)
    {
        rightArray = (TeamRecord *)malloc(rightSize * sizeof(TeamRecord));
        if (rightArray == NULL)
        {
            printf("Memory allocation failed in mergeTeamSegmentsByAverage (rightArray).\n");
            free(leftArray);
            return;
        }
        for (int index = 0; index < rightSize; ++index)
        {
            rightArray[index] = teams[middleIndex + 1 + index];
        }
    }

    int leftPosition = 0;
    int rightPosition = 0;
    int writePosition = startIndex;

    while ((leftArray != NULL && leftPosition < leftSize) && (rightArray != NULL && rightPosition < rightSize))
    {
        if (leftArray[leftPosition].averageBattingStrikeRate >= rightArray[rightPosition].averageBattingStrikeRate)
        {
            teams[writePosition++] = leftArray[leftPosition++];
        }
        else
        {
            teams[writePosition++] = rightArray[rightPosition++];
        }
    }

    while (leftArray != NULL && leftPosition < leftSize)
    {
        teams[writePosition++] = leftArray[leftPosition++];
    }

    while (rightArray != NULL && rightPosition < rightSize)
    {
        teams[writePosition++] = rightArray[rightPosition++];
    }

    free(leftArray);
    free(rightArray);
}

void sortRoleIndexArrayByPerformance(PlayerRecord players[], int indexArray[], int startIndex, int endIndex)
{
    if (startIndex < endIndex)
    {
        int middleIndex = startIndex + (endIndex - startIndex) / 2;
        sortRoleIndexArrayByPerformance(players, indexArray, startIndex, middleIndex);
        sortRoleIndexArrayByPerformance(players, indexArray, middleIndex + 1, endIndex);
        mergeRoleIndexSegments(players, indexArray, startIndex, middleIndex, endIndex);
    }
}

void buildTeamsFromPlayers(TeamRecord teamRecords[], int *loadedTeamCount, PlayerRecord playerRecords[], int loadedPlayerCount)
{
    *loadedTeamCount = 0;
    for (int index = 0; index < teamCount; index++)
    {
        teamRecords[index].id = index + 1;
        strncpy(teamRecords[index].teamName, teams[index], MAX_NAME_LENGTH - 1);
        teamRecords[index].teamName[MAX_NAME_LENGTH - 1] = '\0';
        teamRecords[index].totalPlayers = 0;
        teamRecords[index].averageBattingStrikeRate = 0.0f;
        teamRecords[index].batsmanCount = 0;
        teamRecords[index].bowlerCount = 0;
        teamRecords[index].allrounderCount = 0;
        (*loadedTeamCount)++;
    }
    for (int count = 0; count < loadedPlayerCount; count++)
    {
        for (int index = 0; index < *loadedTeamCount; index++)
        {
            if (strcmp(playerRecords[count].teamName, teamRecords[index].teamName) == 0)
            {
                int current = teamRecords[index].totalPlayers;
                if (current < MAX_PLAYERS_PER_TEAM)
                {
                    teamRecords[index].playerIndexes[current] = count;
                    teamRecords[index].totalPlayers++;
                }
                if (strcmp(playerRecords[count].roleName, "Batsman") == 0)
                {
                    int position = teamRecords[index].batsmanCount;
                    if (position < MAX_PLAYERS_PER_TEAM)
                    {
                        teamRecords[index].batsmanIndexes[position] = count;
                        teamRecords[index].batsmanCount++;
                    }
                }
                else if (strcmp(playerRecords[count].roleName, "Bowler") == 0)
                {
                    int position = teamRecords[index].bowlerCount;
                    if (position < MAX_PLAYERS_PER_TEAM)
                    {
                        teamRecords[index].bowlerIndexes[position] = count;
                        teamRecords[index].bowlerCount++;
                    }
                }
                else
                {
                    int position = teamRecords[index].allrounderCount;
                    if (position < MAX_PLAYERS_PER_TEAM)
                    {
                        teamRecords[index].allrounderIndexes[position] = count;
                        teamRecords[index].allrounderCount++;
                    }
                }
                break;
            }
        }
    }
    for (int index = 0; index < *loadedTeamCount; index++)
    {
        float strikeSum = 0.0f;
        int eligible = 0;
        for (int teamIndex = 0; teamIndex < teamRecords[index].totalPlayers; teamIndex++)
        {
            int playerIndex = teamRecords[index].playerIndexes[teamIndex];
            if (strcmp(playerRecords[playerIndex].roleName, "Batsman") == 0 || strcmp(playerRecords[playerIndex].roleName, "All-rounder") == 0)
            {
                strikeSum += playerRecords[playerIndex].strikeRate;
                eligible++;
            }
        }
        if (eligible > 0) 
        {
            teamRecords[index].averageBattingStrikeRate = strikeSum / eligible;
        }
        else 
        {
            teamRecords[index].averageBattingStrikeRate = 0.0f;
        }
        if (teamRecords[index].batsmanCount > 0) 
        {
            sortRoleIndexArrayByPerformance(playerRecords, teamRecords[index].batsmanIndexes, 0, teamRecords[index].batsmanCount - 1);
        }
        if (teamRecords[index].bowlerCount > 0) 
        {
            sortRoleIndexArrayByPerformance(playerRecords, teamRecords[index].bowlerIndexes, 0, teamRecords[index].bowlerCount - 1);
        }
        if (teamRecords[index].allrounderCount > 0) 
        {
            sortRoleIndexArrayByPerformance(playerRecords, teamRecords[index].allrounderIndexes, 0, teamRecords[index].allrounderCount - 1);
        }
    }
}

int findTeamIndexById(TeamRecord teamRecords[], int teamId, int loadedTeamCount)
{
    int low = 0;
    int high = loadedTeamCount - 1;
    while (low <= high)
    {
        int middle = low + (high - low) / 2;
        if (teamRecords[middle].id == teamId) 
        {
            return middle;
        }
        if (teamRecords[middle].id < teamId) 
        {
            low = middle + 1;
        }
        else 
        {
            high = middle - 1;
        }
    }
    return -1;
}

void showPlayersForTeamId(TeamRecord teams[], PlayerRecord players[], int teamId, int loadedTeamCount)
{
    int foundIndex = findTeamIndexById(teams, teamId, loadedTeamCount);
    if (foundIndex == -1)
    {
        printf("Team not found.\n");
        return;
    }
    printf("\nPlayers of Team %s:\n", teams[foundIndex].teamName);
    printf("================================================================================================================\n");
    printf("%-5s %-20s %-15s %-10s %-10s %-10s %-10s %-10s %-10s\n", "ID", "Name", "Role", "Runs", "Avg", "SR", "Wkts", "ER", "Perf.Score");
    printf("================================================================================================================\n");
    for (int teamPlayerPos = 0; teamPlayerPos < teams[foundIndex].totalPlayers; teamPlayerPos++)
    {
        int playerIndex = teams[foundIndex].playerIndexes[teamPlayerPos];
        printf("%-5d %-20s %-15s %-10d %-10.2f %-10.2f %-10d %-10.2f %-10.2f\n",
               players[playerIndex].id,
               players[playerIndex].fullName,
               players[playerIndex].roleName,
               players[playerIndex].runs,
               players[playerIndex].battingAvg,
               players[playerIndex].strikeRate,
               players[playerIndex].wickets,
               players[playerIndex].economy,
               players[playerIndex].performanceScore);
    }
    printf("================================================================================================================\n");
    printf("Total Players: %d\n", teams[foundIndex].totalPlayers);
    printf("Average Batting Strike Rate: %0.2f\n\n", teams[foundIndex].averageBattingStrikeRate);
}

void sortTeamsByAverageStrikeRate(TeamRecord teams[], int startIndex, int endIndex)
{
    if (startIndex < endIndex)
    {
        int middleIndex = startIndex + (endIndex - startIndex) / 2;
        sortTeamsByAverageStrikeRate(teams, startIndex, middleIndex);
        sortTeamsByAverageStrikeRate(teams, middleIndex + 1, endIndex);
        mergeTeamSegmentsByAverage(teams, startIndex, middleIndex, endIndex);
    }
}

void showTeamsOrderedByStrikeRate(TeamRecord teams[], int loadedTeamCount)
{
    TeamRecord *copyOfTeams = (TeamRecord *)malloc(loadedTeamCount * sizeof(TeamRecord));
    if (copyOfTeams == NULL)
    {
        printf("Memory allocation failed in showTeamsOrderedByStrikeRate.\n");
        return;
    }

    for (int copyIndex = 0; copyIndex < loadedTeamCount; copyIndex++)
    {
        copyOfTeams[copyIndex] = teams[copyIndex];
    }

    sortTeamsByAverageStrikeRate(copyOfTeams, 0, loadedTeamCount - 1);

    printf("\nTeams Sorted by Average Batting Strike Rate\n\n");
    printf("=====================================================================\n");
    printf("%-10s %-25s %-15s %-10s\n", "ID", "Team Name", "Avg Bat SR", "Players");
    printf("=====================================================================\n");

    for (int displayIndex = 0; displayIndex < loadedTeamCount; displayIndex++)
    {
        printf("%-10d %-25s %-15.2f %-10d\n",
               copyOfTeams[displayIndex].id,
               copyOfTeams[displayIndex].teamName,
               copyOfTeams[displayIndex].averageBattingStrikeRate,
               copyOfTeams[displayIndex].totalPlayers);
    }

    printf("=====================================================================\n");

    free(copyOfTeams);
}

void showTopKPlayersForRoleInTeam(TeamRecord teams[], PlayerRecord players[], int loadedTeamCount)
{
    printf("\nEnter Team ID: ");
    int inputTeamId = readValidatedInteger();
    int foundTeamIndex = findTeamIndexById(teams, inputTeamId, loadedTeamCount);
    if (foundTeamIndex == -1)
    {
        printf("Error: Invalid Team ID. Team does not exist.\n");
        return;
    }
    printf("Enter Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    int roleChoice = readValidatedInteger();
    if (roleChoice < 1 || roleChoice > 3)
    {
        printf("Error: Invalid role choice.\n");
        return;
    }
    printf("Enter number of players: ");
    int numberOfPlayers = readValidatedInteger();
    int *selectedList = NULL;
    int selectedCount = 0;
    char displayRoleName[MAX_NAME_LENGTH];
    if (roleChoice == 1)
    {
        selectedList = teams[foundTeamIndex].batsmanIndexes;
        selectedCount = teams[foundTeamIndex].batsmanCount;
        strncpy(displayRoleName, "Batsmen", MAX_NAME_LENGTH);
    }
    else if (roleChoice == 2)
    {
        selectedList = teams[foundTeamIndex].bowlerIndexes;
        selectedCount = teams[foundTeamIndex].bowlerCount;
        strncpy(displayRoleName, "Bowlers", MAX_NAME_LENGTH);
    }
    else
    {
        selectedList = teams[foundTeamIndex].allrounderIndexes;
        selectedCount = teams[foundTeamIndex].allrounderCount;
        strncpy(displayRoleName, "All-rounders", MAX_NAME_LENGTH);
    }
    if (selectedCount == 0)
    {
        printf("No players of this role in the team.\n");
        return;
    }
    if (numberOfPlayers > selectedCount)
    {
        printf("Error: Only %d %s present in this team.\n\n", selectedCount, displayRoleName);
        return;
    }
    printf("\nTop %d %s of Team %s:\n", numberOfPlayers, displayRoleName, teams[foundTeamIndex].teamName);
    printf("=========================================================================================================\n");
    printf("%-5s %-20s %-15s %-10s %-10s %-10s %-10s %-10s %-10s\n", "ID", "Name", "Role", "Runs", "Avg", "SR", "Wkts", "ER", "Perf.Score");
    printf("=========================================================================================================\n");
    for (int resultPosition = 0; resultPosition < numberOfPlayers; resultPosition++)
    {
        int playerIndex = selectedList[resultPosition];
        printf("%-5d %-20s %-15s %-10d %-10.2f %-10.2f %-10d %-10.2f %-10.2f\n",
               players[playerIndex].id,
               players[playerIndex].fullName,
               players[playerIndex].roleName,
               players[playerIndex].runs,
               players[playerIndex].battingAvg,
               players[playerIndex].strikeRate,
               players[playerIndex].wickets,
               players[playerIndex].economy,
               players[playerIndex].performanceScore);
    }
}

void swapHeapNodes(TeamPlayerHeapNode *firstNode, TeamPlayerHeapNode *secondNode)
{
    TeamPlayerHeapNode temporaryNode = *firstNode;
    *firstNode = *secondNode;
    *secondNode = temporaryNode;
}

void heapSiftDown(TeamPlayerHeapNode heap[], int heapSize, int currentIndex, PlayerRecord players[])
{
    int largest = currentIndex;
    int leftChild = 2 * currentIndex + 1;
    int rightChild = 2 * currentIndex + 2;
    if (leftChild < heapSize && players[heap[leftChild].playerIndex].performanceScore > players[heap[largest].playerIndex].performanceScore)
    {
        largest = leftChild;
    }
    if (rightChild < heapSize && players[heap[rightChild].playerIndex].performanceScore > players[heap[largest].playerIndex].performanceScore)
    {
        largest = rightChild;
    }
    if (largest != currentIndex)
    {
        swapHeapNodes(&heap[currentIndex], &heap[largest]);
        heapSiftDown(heap, heapSize, largest, players);
    }
}

void heapInsertNode(TeamPlayerHeapNode heap[], int *heapSize, TeamPlayerHeapNode nodeToInsert, PlayerRecord players[])
{
    int insertPosition = *heapSize;
    int current = insertPosition;
    heap[insertPosition] = nodeToInsert;
    (*heapSize)++;
    while (current > 0)
    {
        int parent = (current - 1) / 2;
        if (players[heap[current].playerIndex].performanceScore > players[heap[parent].playerIndex].performanceScore)
        {
            swapHeapNodes(&heap[current], &heap[parent]);
            current = parent;
        }
        else 
        {
            break;
        }
    }
}

TeamPlayerHeapNode heapPopMaxNode(TeamPlayerHeapNode heap[], int *heapSize, PlayerRecord players[])
{
    TeamPlayerHeapNode root = heap[0];
    heap[0] = heap[*heapSize - 1];
    (*heapSize)--;
    if (*heapSize > 0) 
    {
        heapSiftDown(heap, *heapSize, 0, players);
    }
    return root;
}

void showAllPlayersByRoleAcrossTeams(TeamRecord teams[], PlayerRecord players[], int roleChoice, int loadedTeamCount)
{
    char roleLabel[MAX_NAME_LENGTH];
    if (roleChoice == 1) 
    {
        strncpy(roleLabel, "Batsmen", MAX_NAME_LENGTH);
    }
    else if (roleChoice == 2) 
    {
        strncpy(roleLabel, "Bowlers", MAX_NAME_LENGTH);
    }
    else 
    {
        strncpy(roleLabel, "All-rounders", MAX_NAME_LENGTH);
    }
    int totalRolePlayers = 0;
    int initialHeapCapacity = loadedTeamCount > 0 ? loadedTeamCount : 1;
    TeamPlayerHeapNode *mergeHeap = (TeamPlayerHeapNode *)malloc(initialHeapCapacity * sizeof(TeamPlayerHeapNode));
    if (mergeHeap == NULL)
    {
        printf("Memory allocation failed.\n");
        return;
    }
    int heapSize = 0;
    for (int teamLoop = 0; teamLoop < loadedTeamCount; teamLoop++)
    {
        int *roleList = NULL;
        int roleCountForThisTeam = 0;
        if (roleChoice == 1) 
        {
            roleList = teams[teamLoop].batsmanIndexes; 
            roleCountForThisTeam = teams[teamLoop].batsmanCount; 
        }
        else if (roleChoice == 2) 
        { 
            roleList = teams[teamLoop].bowlerIndexes; 
            roleCountForThisTeam = teams[teamLoop].bowlerCount; 
        }
        else 
        { 
            roleList = teams[teamLoop].allrounderIndexes; 
            roleCountForThisTeam = teams[teamLoop].allrounderCount; 
        }
        if (roleCountForThisTeam > 0)
        {
            totalRolePlayers += roleCountForThisTeam;
            TeamPlayerHeapNode node;
            node.playerIndex = roleList[0];
            node.teamIndex = teamLoop;
            node.indexInRoleList = 0;
            heapInsertNode(mergeHeap, &heapSize, node, players);
        }
    }
    if (totalRolePlayers == 0)
    {
        printf("No %s found across any team.\n", roleLabel);
        free(mergeHeap);
        return;
    }
    printf("\nAll %s Across All Teams (sorted by performance):\n", roleLabel);
    printf("==================================================================================================================\n");
    printf("%-5s %-20s %-15s %-15s %-10s %-10s %-10s %-10s %-10s\n", "ID", "Name", "Team", "Role", "Runs", "Avg", "SR", "Wkts", "Perf.Score");
    printf("==================================================================================================================\n");
    while (heapSize > 0)
    {
        TeamPlayerHeapNode bestNode = heapPopMaxNode(mergeHeap, &heapSize, players);
        int playerIndex = bestNode.playerIndex;
        int teamIndex = bestNode.teamIndex;
        int nextIndex = bestNode.indexInRoleList + 1;
        printf("%-5d %-20s %-15s %-15s %-10d %-10.2f %-10.2f %-10d %-10.2f\n",
               players[playerIndex].id,
               players[playerIndex].fullName,
               teams[teamIndex].teamName,
               players[playerIndex].roleName,
               players[playerIndex].runs,
               players[playerIndex].battingAvg,
               players[playerIndex].strikeRate,
               players[playerIndex].wickets,
               players[playerIndex].performanceScore);
        int *roleList = NULL;
        int roleCountForThisTeam = 0;
        if (roleChoice == 1) 
        { 
            roleList = teams[teamIndex].batsmanIndexes; 
            roleCountForThisTeam = teams[teamIndex].batsmanCount; 
        }
        else if (roleChoice == 2) 
        { 
            roleList = teams[teamIndex].bowlerIndexes; 
            roleCountForThisTeam = teams[teamIndex].bowlerCount; 
        }
        else 
        { 
            roleList = teams[teamIndex].allrounderIndexes; 
            roleCountForThisTeam = teams[teamIndex].allrounderCount; 
        }
        if (nextIndex < roleCountForThisTeam)
        {
            TeamPlayerHeapNode newNode;
            newNode.playerIndex = roleList[nextIndex];
            newNode.teamIndex = teamIndex;
            newNode.indexInRoleList = nextIndex;
            heapInsertNode(mergeHeap, &heapSize, newNode, players);
        }
    }
    printf("==================================================================================================================\n");
    free(mergeHeap);
}

void addNewPlayerToTeam(TeamRecord teams[], PlayerRecord players[], int *loadedPlayerCount, int loadedTeamCount)
{
    if (*loadedPlayerCount >= MAX_PLAYERS_PER_TEAM * loadedTeamCount)
    {
        printf("Cannot add more players: maximum total capacity reached.\n");
        return;
    }
    printf("\nEnter Team ID to add player: ");
    int chosenTeamId = readValidatedInteger();
    int teamIndex = findTeamIndexById(teams, chosenTeamId, loadedTeamCount);
    if (teamIndex == -1)
    {
        printf("Error: Team with ID %d not found.\n", chosenTeamId);
        return;
    }
    if (teams[teamIndex].totalPlayers >= MAX_PLAYERS_PER_TEAM)
    {
        printf("Cannot add more players to team %s: team is full (max %d players).\n", teams[teamIndex].teamName, MAX_PLAYERS_PER_TEAM);
        return;
    }
    PlayerRecord newRecord;
    printf("Enter Player ID: ");
    newRecord.id = readValidatedInteger();
    printf("Enter Name: ");
    if (fgets(newRecord.fullName, sizeof(newRecord.fullName), stdin) == NULL) 
    { 
        clearerr(stdin); 
        printf("Failed to read name.\n"); 
        return; 
    }
    int lengthName = (int)strlen(newRecord.fullName);
    if (lengthName > 0 && newRecord.fullName[lengthName - 1] == '\n') 
    {
        newRecord.fullName[lengthName - 1] = '\0';
    }
    printf("Enter Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    int roleChoice = readValidatedInteger();
    if (roleChoice == 1) 
    {
        strncpy(newRecord.roleName, "Batsman", MAX_NAME_LENGTH);
    }
    else if (roleChoice == 2) 
    {
        strncpy(newRecord.roleName, "Bowler", MAX_NAME_LENGTH);
    }
    else 
    {
        strncpy(newRecord.roleName, "All-rounder", MAX_NAME_LENGTH);
    }
    printf("Enter Total Runs (integer): ");
    newRecord.runs = readValidatedInteger();
    printf("Enter Batting Average (float): ");
    newRecord.battingAvg = readValidatedFloat();
    printf("Enter Strike Rate (float): ");
    newRecord.strikeRate = readValidatedFloat();
    printf("Enter Wickets (integer): ");
    newRecord.wickets = readValidatedInteger();
    printf("Enter Economy Rate (float): ");
    newRecord.economy = readValidatedFloat();
    strncpy(newRecord.teamName, teams[teamIndex].teamName, MAX_NAME_LENGTH - 1);
    newRecord.teamName[MAX_NAME_LENGTH - 1] = '\0';
    if (strcmp(newRecord.roleName, "Batsman") == 0)
    {
        newRecord.performanceScore = (newRecord.battingAvg * newRecord.strikeRate) / 100.0f;
    }
    else if (strcmp(newRecord.roleName, "Bowler") == 0)
    {
        newRecord.performanceScore = (newRecord.wickets * 2) + (100 - newRecord.economy);
    }
    else
    {
        newRecord.performanceScore = ((newRecord.battingAvg * newRecord.strikeRate) / 100.0f) + (newRecord.wickets * 2);
    }
    int appendIndex = *loadedPlayerCount;
    players[appendIndex] = newRecord;
    (*loadedPlayerCount)++;
    int teamPlayerPosition = teams[teamIndex].totalPlayers;
    teams[teamIndex].playerIndexes[teamPlayerPosition] = appendIndex;
    teams[teamIndex].totalPlayers++;
    if (strcmp(newRecord.roleName, "Batsman") == 0)
    {
        int pos = teams[teamIndex].batsmanCount;
        if (pos < MAX_PLAYERS_PER_TEAM) 
        { 
            teams[teamIndex].batsmanIndexes[pos] = appendIndex; 
            teams[teamIndex].batsmanCount++; 
            sortRoleIndexArrayByPerformance(players, teams[teamIndex].batsmanIndexes, 0, teams[teamIndex].batsmanCount - 1); 
        }
    }
    else if (strcmp(newRecord.roleName, "Bowler") == 0)
    {
        int pos = teams[teamIndex].bowlerCount;
        if (pos < MAX_PLAYERS_PER_TEAM) 
        { 
            teams[teamIndex].bowlerIndexes[pos] = appendIndex; 
            teams[teamIndex].bowlerCount++; 
            sortRoleIndexArrayByPerformance(players, teams[teamIndex].bowlerIndexes, 0, teams[teamIndex].bowlerCount - 1); 
        }
    }
    else
    {
        int pos = teams[teamIndex].allrounderCount;
        if (pos < MAX_PLAYERS_PER_TEAM) 
        { 
            teams[teamIndex].allrounderIndexes[pos] = appendIndex; 
            teams[teamIndex].allrounderCount++; 
            sortRoleIndexArrayByPerformance(players, teams[teamIndex].allrounderIndexes, 0, teams[teamIndex].allrounderCount - 1); 
        }
    }
    float strikeSum = 0.0f;
    int eligibleCount = 0;
    for (int pos = 0; pos < teams[teamIndex].totalPlayers; ++pos)
    {
        int pIndex = teams[teamIndex].playerIndexes[pos];
        if (strcmp(players[pIndex].roleName, "Batsman") == 0 || strcmp(players[pIndex].roleName, "All-rounder") == 0)
        {
            strikeSum += players[pIndex].strikeRate;
            eligibleCount++;
        }
    }
    if (eligibleCount > 0) 
    {
        teams[teamIndex].averageBattingStrikeRate = strikeSum / eligibleCount;
    }
    else 
    {
        teams[teamIndex].averageBattingStrikeRate = 0.0f;
    }
    printf("Player added successfully to Team %s!\n", teams[teamIndex].teamName);
}

int main()
{
    int initialPlayerCapacity = playerCount + MAX_PLAYERS_PER_TEAM * teamCount;
    PlayerRecord *playersList = (PlayerRecord *)malloc(initialPlayerCapacity * sizeof(PlayerRecord));
    if (playersList == NULL)
    {
        fprintf(stderr, "Memory allocation failed for playersList\n");
        return 1;
    }

    TeamRecord *teamsList = (TeamRecord *)malloc(teamCount * sizeof(TeamRecord));
    if (teamsList == NULL)
    {
        fprintf(stderr, "Memory allocation failed for teamsList\n");
        free(playersList);
        return 1;
    }

    int loadedPlayerCount = 0;
    int loadedTeamCount = 0;

    populatePlayersFromData(playersList, &loadedPlayerCount);
    buildTeamsFromPlayers(teamsList, &loadedTeamCount, playersList, loadedPlayerCount);

    int menuChoice = 0;

    do
    {
        printf("\n==============================================================================\n");
        printf("ICC ODI Player Performance Analyzer\n");
        printf("==============================================================================\n\n");
        printf("1. Add Player to Team\n");
        printf("2. Display Players of a Specific Team\n");
        printf("3. Display Teams by Average Batting Strike Rate\n");
        printf("4. Display Top K Players of a Specific Team by Role\n");
        printf("5. Display All Players of a Specific Role Across All Teams\n");
        printf("6. Exit\n");
        printf("==============================================================================\n");
        printf("Enter your choice: ");

        menuChoice = readValidatedInteger();

        if (menuChoice == 1)
        {
            addNewPlayerToTeam(teamsList, playersList, &loadedPlayerCount, loadedTeamCount);
        }
        else if (menuChoice == 2)
        {
            printf("\nEnter Team ID: ");
            int inputTeamId = readValidatedInteger();
            showPlayersForTeamId(teamsList, playersList, inputTeamId, loadedTeamCount);
        }
        else if (menuChoice == 3)
        {
            showTeamsOrderedByStrikeRate(teamsList, loadedTeamCount);
        }
        else if (menuChoice == 4)
        {
            showTopKPlayersForRoleInTeam(teamsList, playersList, loadedTeamCount);
        }
        else if (menuChoice == 5)
        {
            printf("\nEnter Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
            int roleChoice = readValidatedInteger();
            if (roleChoice < 1 || roleChoice > 3)
                printf("Error: Invalid role choice.\n");
            else
                showAllPlayersByRoleAcrossTeams(teamsList, playersList, roleChoice, loadedTeamCount);
        }
        else if (menuChoice == 6)
        {
            printf("\nExiting Program...\n");
        }
        else
        {
            printf("\nPlease Enter Valid Option\n");
        }

    } while (menuChoice != 6);

    free(playersList);
    free(teamsList);

    return 0;
}
