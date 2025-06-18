#include <bits/stdc++.h>
// #include <algorithm>
// #include <iomanip>
// #include <vector>
// #include
using namespace std;

int N, g[11], k[11], m[11], M, s[501], e[501], cnt[501], latency[11][501], a, b;

void get_argument()
{
    cin >> N;
    for (int i = 1; i <= N; i++)
        cin >> g[i] >> k[i] >> m[i];
    cin >> M;
    for (int i = 1; i <= M; i++)
    {
        cin >> s[i] >> e[i] >> cnt[i];
    }
    for (int i = 1; i <= N; i++)
    {
        for (int j = 1; j <= M; j++)
        {
            cin >> latency[i][j];
        }
    }
    cin >> a >> b;
}

void solution()
{
    for(int i = 1; i <= M; i++)
    {
        int Ti = 0, serverj = i % N + 1;
        int Bj = min(1000, (m[serverj] - b) / a);
        Ti = (int)ceil(1.0 * cnt[i] / Bj);
        int timej = s[i];

        cout << Ti << "\n";  // Ti must less than 300, there won't cost a problem

        for(int k = 1; k <= Ti; k++)
        {
            cout << timej << " " << serverj << " " << 1 << " " << Bj << " ";
            timej += latency[serverj][i] + 1;
        }
        cout << "\n";
    }
}

int main()
{
    get_argument();
    solution();
    
    return 0;
}
// g++ demo.cpp -o demo; get-Content .\sample\data.in |demo.exe > output.txt