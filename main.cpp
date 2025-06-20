#include <bits/stdc++.h>
using namespace std;
// 单个npu能处理的样本的大小范围是[40, 190]
// 单个npu完成上述大小范围的样本所需时间的范围是[2, 14]
int N, g[11], k[11], m[11], M, latency[11][501], a, b, server_index[501][11], NPU_size[11][11][200001], request_size[11];
struct Plan
{
    int timej, serverj, NPUj, Bj;
    // Plan() : timej(0), serverj(0), NPUj(0), Bj(0) {}
};
vector<Plan> ans[501]; // 这个ans的下标是用户的真实id
struct User
{
    int id, s, e, cnt;
    User() : id(0), s(0), e(0), cnt(0) {}
    bool operator<(const User &other)
    {
        if (s != other.s)
            return s < other.s;
        else
            return cnt < other.cnt;
    }
} user[501];

void get_argument_initial()
{
    cin >> N;
    for (int i = 1; i <= N; i++)
        cin >> g[i] >> k[i] >> m[i];
    cin >> M;
    for (int i = 1; i <= M; i++)
    {
        cin >> user[i].s >> user[i].e >> user[i].cnt;
        user[i].id = i;
    }
    for (int i = 1; i <= N; i++)
    {
        for (int j = 1; j <= M; j++)
        {
            cin >> latency[i][j];
        }
    }
    cin >> a >> b;

    for (int i = 1; i <= N; i++)
        for (int j = 1; j <= g[i]; j++)
            for (int k = 0; k <= 200000; k++)
                NPU_size[i][j][k] = (m[i] - b) / a; // 确实应该向下取整，

    for (int i = 1; i <= N; i++)
    {
        request_size[i] = min(NPU_size[i][1][1], 1000); // 表示应该向第i个服务器的NPU放入多大的样本数量
    }
}

int request_time(int size, int server, int user)
{
    int id = ::user[user].id;
    return latency[server][id] + (int)ceil(sqrt(size) / k[server]);
}

void sort_server()
{
    sort(user + 1, user + 1 + M);

    for (int i = 1; i <= M; i++)
    {

        for (int j = 1; j <= N; j++)
        {
            server_index[i][j] = j; // 存的是服务器的下标
        }
    }
    // cout << endl << "test" << endl;
    for (int i = 1; i <= M; i++)
    {
        sort(server_index[i] + 1, server_index[i] + N + 1, [i](int server_index1, int server_index2)
             { return double(request_time(request_size[server_index1], server_index1, i)) / request_size[server_index1] <
                      double(request_time(request_size[server_index2], server_index2, i)) / request_size[server_index2]; });
        // cout << "user_" << i << ":";
        // for (int j = 1; j <= N; j++)
        // {
        //     cout << server_index[i][j] << " ";
        // }
        // cout << endl;
    }
    return;
}

void solution()
{
    for (int i = 1; i <= M; i++)
    {
        int id = user[i].id, Fast_Time = 0x3f3f3f3f;

        for (int j = 1; j <= N; j++)
        {
            int Ti = (int)ceil(1.0 * user[i].cnt / request_size[j]);
            for (int k = 1; k <= g[j]; k++)
            {
                vector<Plan> plan;
                int cnt = user[i].cnt;
                int timej = user[i].s;
                int count = 0;
                while (count != Ti)
                {
                    int size = min(request_size[j], cnt);
                    bool flag = 1;
                    for (int p = timej + latency[j][id]; p <= timej + request_time(size, j, i) - 1; p++)
                    {
                        if (NPU_size[j][k][p] < size)
                        {
                            flag = 0;
                            timej = p + 1 - latency[j][id];
                            break;
                        }
                    }
                    if (flag)
                    {

                        plan.push_back({timej, j, k, size});
                        count++;
                        cnt -= size;
                        timej = max(timej + request_time(size, j, i) - latency[j][id], timej + latency[j][id] + 1);
                    } // 下次发送时间          //处理完成的点减去发送所需时间              至少下次发送的时间
                }
                if (plan.back().timej + request_time(plan.back().Bj, j, i) < Fast_Time)
                {
                    Fast_Time = plan.back().timej + request_time(plan.back().Bj, j, i);
                    ans[id] = plan;
                }
            }
        }

        for (auto &j : ans[id])
        {
            for (int p = j.timej + latency[j.serverj][id]; p <= j.timej + request_time(j.Bj, j.serverj, i) - 1; p++)
            {
                NPU_size[j.serverj][j.NPUj][p] -= j.Bj;
            }
        }
    }

    for (int i = 1; i <= M; i++)
    {
        cout << ans[i].size() << "\n"; // Ti must less than 300, there won't cost a problem

        for (auto j : ans[i])
            cout << j.timej << " " << j.serverj << " " << j.NPUj << " " << j.Bj << " ";
        // for (int j = 1; j <= ans[i].size() - 1; j++)
        // {
        //     if (ans[i][j].timej - ans[i][j - 1].timej >= latency[ans[i][j - 1].serverj][i] + 1)
        //         cout << "OK " << latency[ans[i][j - 1].serverj][i] + 1  << " ";
        //     else cout <<"Wrong ";
        // }
        cout << "\n";
    }
}

void monitor_NPU_size()
{
    ofstream out("monitor.txt");
    for (int i = 1; i <= N; i++)
    {
        for (int j = 1; j <= g[i]; j++)
        {
            for (int k = 0; k <= 100000; k++)
            {
                out << NPU_size[i][j][k] << " ";
            }
            out << "\n";
        }
    }
    out.close();
}

int main()
{
    get_argument_initial();
    sort_server();
    solution();

    monitor_NPU_size();
    return 0;
}
// g++ main.cpp -std=c++11 -o main; get-Content .\sample\data.in | main.exe > output.txt