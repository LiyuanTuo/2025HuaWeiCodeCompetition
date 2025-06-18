#include <bits/stdc++.h>
using namespace std;

int N, g[11], k[11], m[11], M, s[501], e[501], cnt[501], latency[11][501], a, b;
int which_gpu[11]; // which_gpu[i]用于表示第i个服务器应该让哪一个gpu处理传送至服务器i的请求
//int que[11][11][200000];
void get_argument_initial()
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
    for(int i = 1; i <= N; i++)which_gpu[i] = 1;// initial which_gpu
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
            cout << timej << " " << serverj << " " << which_gpu[serverj] << " " << min(Bj, cnt[i]) << " ";

            timej += latency[serverj][i] + 1;
            cnt[i] -= min(Bj, cnt[i]);
            which_gpu[serverj] = which_gpu[serverj] % g[serverj] + 1;
        }
        cout << "\n";
    }
}

int main()
{
    
    get_argument_initial();
    solution();
    return 0;
}
// g++ main.cpp -o main; get-Content .\sample\data.in | main.exe > output.txt