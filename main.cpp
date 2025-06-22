#include <bits/stdc++.h>
using namespace std;
// 单个npu能处理的样本的大小范围是[40, 190]
// 单个npu完成上述大小范围的样本所需时间的范围是[2, 14]                           //只能说明有极端数据，也即请求非常多，NPU又不够多的极端数据
// 传输时间范围是[10, 20]                                                      //震惊？？？10万invalid output 20万是success
int N, g[11], k[11], m[11], M, latency[11][501], a, b, server_index[501][11], NPU_size[11][11][200001], request_size[11];
int receive_process[11][11][200001];
struct Plan
{
    int timej, serverj, NPUj, Bj, process_start;
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
        for (int j = 1; j <= g[i]; j++)  //初始化
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
            for (int k = 1; k <= g[j]; k++)
            {
                int Ti = (int)ceil(1.0 * user[i].cnt / request_size[j]);
                vector<Plan> plan;
                int cnt = user[i].cnt;
                int timej = user[i].s, process_start = timej + latency[j][id];
                int count = 0;
                while (count != Ti)
                {
                    int size = min(request_size[j], cnt);
                    int time_process = request_time(size, j, i) - latency[j][id];
                    
                    bool flag = 1;
                    for (int p = 0; p < time_process; p++)
                        if (NPU_size[j][k][process_start + p] < size)
                        {
                            flag = 0;
                            process_start = process_start + p + 1;
                            break;
                        }

                    if (flag)
                    {   //process_start存储的是该NPU开始处理当前这个请求的时间
                        //也即是服务器接收到这个请求时间的上界 timej + latency[j][id] 是接收到这个请求时间的下界
                        for(int q = 0; q <= time_process - 1; q++)
                        {
                            NPU_size[j][k][process_start + q] -= size;  //更新这个NPU_size
                        }
                        int receive_time = timej + latency[j][id];
                        for(int q = timej + latency[j][id]; q <= process_start - 1; q++)
                        {
                            if(receive_process[j][k][q])
                            {
                                receive_time = q + 1; // 确定receive_time
                            }
                        }   
                        receive_process[j][k][receive_time]++; // 更新receive_process
                        // receive_time 存储的就是这个NPU收到这个请求的时间
                        //于是timej就是这个请求正确发送的最早时间，并且再次初始化process_start
                        timej = receive_time - latency[j][id];
                        plan.push_back({timej, j, k, size, process_start});
                        timej = timej + latency[j][id] + 1;//准备下一次请求的发送时间
                        process_start = timej + latency[j][id];
                        count++;
                        cnt -= size;
                    }        
                }
            
                if (plan.back().process_start + request_time(plan.back().Bj, j, i) - latency[j][id] <= Fast_Time)
                {
                    Fast_Time = plan.back().process_start + request_time(plan.back().Bj, j, i) - latency[j][id];
                    ans[id] = plan;
                }
                //还原
                for(Plan& p : plan)
                {
                    receive_process[j][k][p.timej + latency[j][id]]--;
                    int time_process = request_time(p.Bj, j, i) - latency[j][id];
                    for(int q = 0; q <= time_process - 1; q++)
                        NPU_size[j][k][p.process_start + q] += p.Bj;
                }
            }
        }
        
        for (auto j : ans[id])
        {
            for (int k = j.process_start; k <= j.process_start + request_time(j.Bj, j.serverj, i) - latency[j.serverj][id] - 1; k++)
            {
                NPU_size[j.serverj][j.NPUj][k] -= j.Bj;
                //cout << "";
            }

            receive_process[j.serverj][j.NPUj][j.timej + latency[j.serverj][id]]++;
        }
                
    }

    for (int i = 1; i <= M; i++)
    {
        cout << ans[i].size() << "\n"; // Ti must less than 300, there won't cost a problem

        for (auto j : ans[i])
            cout << j.timej << " " << j.serverj << " " << j.NPUj << " " << j.Bj << " ";
        // for (int j = 1; j <= ans[i].size() - 1; j++)
        // {                       danger ans[i].size() - 1
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
            for (int k = 0; k <= 200000; k++)
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