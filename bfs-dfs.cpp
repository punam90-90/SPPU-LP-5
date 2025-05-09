#include <iostream>      // For input/output
#include <vector>        // For using dynamic arrays
#include <queue>         // For BFS
#include <stack>         // For DFS
#include <omp.h>         // OpenMP for parallelism

using namespace std;

const int MAXN = 100005;
vector<int> adj[MAXN];     // Adjacency list to store graph
bool visited[MAXN];        // Track visited nodes

// Function for Parallel Breadth-First Search
void parallel_bfs(int start) {
    queue<int> q;
    q.push(start);
    visited[start] = true;

    while (!q.empty()) {
        int level_size = q.size();        // Number of nodes at current level
        vector<int> current_level;

        // Extract all nodes at the current level
        for (int i = 0; i < level_size; i++) {
            int v = q.front();  // Get the front node
            q.pop();            // Remove it from queue
            cout << v << " ";   // Print the node
            current_level.push_back(v);  // Store for next parallel step
        }

        vector<int> next_level; // Store next level nodes

        // Process current level in parallel
        #pragma omp parallel for
        for (int i = 0; i < current_level.size(); i++) {
            int v = current_level[i];

            // Traverse neighbors of node v
            for (int u : adj[v]) {
                #pragma omp critical   // Only one thread updates visited and next_level at a time
                {
                    if (!visited[u]) {
                        visited[u] = true;
                        next_level.push_back(u);
                    }
                }
            }
        }

        // Push all next level nodes into the queue
        for (int u : next_level)
            q.push(u);
    }
}

// Function for Parallel Depth-First Search
void parallel_dfs(int start) {
    stack<int> s;
    s.push(start);
    visited[start] = true;

    while (!s.empty()) {
        int v = s.top();   // Get top of stack
        s.pop();           // Remove from stack
        cout << v << " ";  // Print node

        vector<int> neighbors; // Temporarily store unvisited neighbors

        // Process neighbors in parallel
        #pragma omp parallel for
        for (int i = 0; i < adj[v].size(); i++) {
            int u = adj[v][i];
            bool needs_push = false;

            // Check and mark visited inside critical block
            #pragma omp critical
            {
                if (!visited[u]) {
                    visited[u] = true;
                    needs_push = true;
                }
            }

            // If not visited before, store to push later
            if (needs_push) {
                #pragma omp critical
                neighbors.push_back(u);
            }
        }

        // Push all neighbors into the stack
        for (int u : neighbors)
            s.push(u);
    }
}

// Reset all visited nodes before new search
void reset_visited(int n) {
    for (int i = 1; i <= n; i++)
        visited[i] = false;
}

// Main function
int main() {
    int n, m;
    cout << "Enter number of nodes and edges: ";
    cin >> n >> m;

    // Check if number of edges is valid
    if (m > n * (n - 1) / 2) {
        cout << "Error: Too many edges.\n";
        return 1;
    }

    // Take edge input
    cout << "Enter edges:\n";
    for (int i = 0; i < m; i++) {
        int u, v;
        cin >> u >> v;
        adj[u].push_back(v);
        adj[v].push_back(u);  // Since the graph is undirected
    }

    // Menu for user
    while (true) {
        cout << "\nChoose an option:\n1. Parallel BFS\n2. Parallel DFS\n3. Exit\nEnter your choice: ";
        int choice;
        cin >> choice;

        if (choice == 3) break; // Exit loop

        cout << "Enter starting node: ";
        int start;
        cin >> start;

        reset_visited(n);  // Clear visited for fresh run

        if (choice == 1) {
            cout << "Running Parallel BFS...\nVisited nodes: ";
            parallel_bfs(start);
        } else if (choice == 2) {
            cout << "Running Parallel DFS...\nVisited nodes: ";
            parallel_dfs(start);
        }

        // Handle disconnected graph components
        for (int i = 1; i <= n; i++) {
            if (!visited[i] && !adj[i].empty()) {
                cout << "\nGraph has disconnected components. Running again from node: " << i << endl;
                if (choice == 1)
                    parallel_bfs(i);
                else
                    parallel_dfs(i);
            }
        }

        cout << endl;
    }

    return 0;
}