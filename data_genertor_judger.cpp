#include <iostream>
#include <math.h>
#include <fstream>
#include <cstdlib>
#include <vector>
#include<ctime>
#include <algorithm>
#include <iomanip>
#include <time.h>
using namespace std;
int N, g[11], k[11], m[11], M, latency[11][501], a, b, NPU_size[11][11][200001], request_size[11], server_index[501][11], NPU_request_count[11][11];

struct NPU_Request_List
{
    int user, B, received_time, flag;
}NPU_request_list[11][11][300001];

struct User
{
    int id, s, e, cnt, end_time, move;
    User() : id(0), s(0), e(0), cnt(0), end_time(0), move(0) {}
} user[501];

struct Plan
{
    int timej, serverj, NPUj, Bj, time_cost;
};
vector<Plan> solution[501];

void data_loader_generator(bool New)
{
    system("g++ not_full_NPU.cpp -lm -Wl,--stack=134217728 -O0 -std=c++11 -static-libstdc++ -static-libgcc -o not_full_NPU");
    system("g++ main.cpp -lm -Wl,--stack=134217728 -O0 -std=c++11 -static-libstdc++ -static-libgcc -o main");
    if (!New)
    {
        ifstream in("sample\\data.in");
        in >> N;
        for (int i = 1; i <= N; i++)
            in >> g[i] >> k[i] >> m[i];
        in >> M;
        for (int i = 1; i <= M; i++)
        {
            in >> user[i].s >> user[i].e >> user[i].cnt;
            user[i].id = i;
            user[i].end_time = 0;
        }
        for (int i = 1; i <= N; i++)
        {
            for (int j = 1; j <= M; j++)
            {
                in >> latency[i][j];
            }
        }
        in >> a >> b;

        for (int i = 1; i <= N; i++)
            for (int j = 1; j <= g[i]; j++) // 初始化
            {
                NPU_request_count[i][j] = 0;
                for (int k = 0; k <= 200000; k++)
                    NPU_size[i][j][k] = m[i]; // 确实应该向下取整，
            }
               

        for (int i = 1; i <= N; i++)
        {
            request_size[i] = min((m[i] - b) / a, 1000); // 表示应该向第i个服务器的NPU放入多大的样本数量
        }
        in.close();
        system("main.exe < .\\sample\\data.in > output.txt");
        system("not_full_NPU.exe < .\\sample\\data.in > not_full_NPU.txt");

        return;
    }

    srand(time(0)); // 固定随机种子

    // 生成服务器种类 N ∈ [1, 10]
    N = rand() % 2 + 1;
    for (int i = 1; i <= N; ++i)
    {
        g[i] = rand() % 2 + 2;       // 1..10  这里要把数据强化一下注意
        k[i] = rand() % 2 + 1;       // 1..5
        m[i] = rand() % 1001 + 1000; // 1000..2000
    }

    // 生成用户数量 M ∈ [1, 500]
    M = rand() % 100 + 401;

    // 为每个用户生成 s, e, cnt
    for (int i = 1; i <= M; ++i)
    {
        user[i].id = i;
        // cnt ∈ [1, 6000]
        user[i].cnt = rand() % 2000 + 4001;
        int maxS = 60000 - user[i].cnt * 5;
        user[i].s = rand() % (maxS + 1);
        user[i].e = user[i].s + 5 * user[i].cnt + rand() % (60000 - user[i].s - 5 * user[i].cnt + 1);
        user[i].end_time = 0;
    }

    // 生成通信延迟 latency[i][j] ∈ [10, 20]
    for (int i = 1; i <= M; ++i)
    {
        for (int j = 1; j <= N; ++j)
        {
            latency[j][i] = rand() % 11 + 10;
        }
    }

    // 生成显存系数 a ∈ [10,20], b ∈ [100,200]
    a = rand() % 5 + 16;   // 10..20
    b = rand() % 101 + 100; // 100..200

    // 写入文件 ./sample/extra_data.in
    ofstream ofs(".//sample//extra_data.in");
    ofs << N << "\n";
    for (int i = 1; i <= N; ++i)
    {
        ofs << g[i] << " " << k[i] << " " << m[i] << "\n";
    }
    ofs << M << "\n";
    for (int i = 1; i <= M; ++i)
    {
        ofs << user[i].s << " " << user[i].e << " " << user[i].cnt << "\n";
    }
    for (int i = 1; i <= N; ++i)
    {
        for (int j = 1; j <= M; ++j)
        {
            ofs << latency[i][j] << (j < M ? " " : "");
        }
        ofs << "\n";
    }
    ofs << a << " " << b << "\n";
    for (int i = 1; i <= N; i++)
        for (int j = 1; j <= g[i]; j++) // 初始化
        {
            NPU_request_count[i][j] = 0;
            for (int k = 0; k <= 200000; k++)
                NPU_size[i][j][k] = m[i]; // 确实应该向下取整，
        }
    for (int i = 1; i <= N; i++)
    {
         request_size[i] = min((m[i] - b) / a, 1000); // 表示应该向第i个服务器的NPU放入多大的样本数量
    }
    ofs.close();
    system("main.exe < .\\sample\\extra_data.in > output.txt");
    system("not_full_NPU.exe < .\\sample\\extra_data.in > not_full_NPU.txt");
}

void brief_check()
{
    for (int i = 1; i <= M; i++)
    {
        if (solution[i][0].timej < user[i].s)
        {
            // cout << solution[i][0].timej << "   " << user[i].s << "\n";
            cout << "User id " << i << "   Invalid User Send Time(Send Time should be smaller than si)";
            return;
        }
    }

    for (int i = 1; i <= M; i++)
    { ////这里必须进行类型转换，否则会出现类型转换的错误
        for (int j = 0; j <= int(solution[i].size()) - 2; j++)
            if (solution[i][j + 1].timej <= solution[i][j].timej)
            {
                cout << "User id " << i << "   Invalid Time Order(timej doesn't become bigger and bigger)";
                return;
            }
    }

    for (int i = 1; i <= M; i++)
    { // 这里必须进行类型转换，否则会出现类型转换的错误
        for (int j = 0; j <= int(solution[i].size()) - 2; j++)
            if (solution[i][j + 1].timej - solution[i][j].timej < latency[solution[i][j].serverj][i])
            {
                cout << "User id " << i << "    Invalid User Send Time(Send Time's gap should be bigger)";
                return;
            }
    }

    for (int i = 1; i <= M; i++)
    {
        int sumBj = 0;
        for (Plan &j : solution[i])
            sumBj += j.Bj;

        if (sumBj != user[i].cnt)
        {
            cout << "User id " << i << "    sum Bj don't equal to cnt";
            return;
        }
    }

    for (int i = 1; i <= M; i++)
        if (solution[i].size() > 300)
        {
            cout << "User id " << i << "    Ti > 300!";
            return;
        }

    for (int i = 1; i <= M; i++)
    {
        for (Plan &j : solution[i])
        {
            if (j.Bj > (m[j.serverj] - b) / a)
            {
                cout << "User id " << i << "    Batchsize Exceeds Memory";
                return;
            }
        }
    }

    for (int i = 1; i <= M; i++)
    {
        for (Plan &j : solution[i])
            if (j.serverj > N || j.serverj <= 0)
            {
                cout << "User id " << i << "    Invalid Server Index(serverj > N or serverj < 1)";
            }

        for (Plan &j : solution[i])
            if (j.NPUj > g[j.serverj] || j.NPUj <= 0)
            {
                cout << "User id " << i << "    Invalid NPU Index(NPUj > g[serverj] or NPUj < 1)";
            }
    }
    cout << "Success"<<endl;
}

double h_x(double x)
{
    return pow(2, -x / 100.0);
}

double p_x(double x)
{
    return pow(2, -x / 200.0);
}

void append_request(int user, int time, int server, int NPU, int B)
{
    NPU_request_count[server][NPU]++;
    NPU_request_list[server][NPU][NPU_request_count[server][NPU]].user = user;
    NPU_request_list[server][NPU][NPU_request_count[server][NPU]].B = B;
    NPU_request_list[server][NPU][NPU_request_count[server][NPU]].received_time = time + latency[server][user];
    NPU_request_list[server][NPU][NPU_request_count[server][NPU]].flag = 0;
    return;
}

bool cmp(NPU_Request_List l1, NPU_Request_List l2)
{
    if (l1.received_time == l2.received_time)
    {
        return l1.user < l2.user;

    }
    else
    {
        return l1.received_time < l2.received_time;
    }
}

int process_time_calculate(int server, int B)
{
    return (int)ceil(sqrt(B) / k[server]);
}

void NPU_request_process()
{
    for (int i = 1; i <= N; i++)
    {
        for (int j = 1; j <= g[i]; j++)
        {
            /*for(int t=1;t<NPU_request_count[i][j];t++)
            {
                if(i==2)
                    cout<<"NPU_request_list["<<i<<"]["<<j<<"]:"<<NPU_request_list[i][j][t].user<<" "<<NPU_request_list[i][j][t].received_time<<" "<<NPU_request_list[i][j][t].B<<endl;
            }*/
            sort(NPU_request_list[i][j] + 1, NPU_request_list[i][j] + 1 + NPU_request_count[i][j], cmp);
            /*for(int t=1;t<NPU_request_count[i][j];t++)
            {
                cout<<"NPU_request_list["<<i<<"]["<<j<<"]:"<<NPU_request_list[i][j][t].user<<" "<<NPU_request_list[i][j][t].received_time<<" "<<NPU_request_list[i][j][t].B<<endl;
            }*/
            for (int t = 0; t <= 200000; t++)
            {
                int temp_flag = 1;
                while (temp_flag == 1)
                {
                    temp_flag = 0;
                    for (int count = 1; count <= NPU_request_count[i][j]; count++)
                    {
                        if (NPU_request_list[i][j][count].flag == 1)
                        {
                            continue;
                        }
                        else
                        {
                            if (NPU_request_list[i][j][count].received_time > t)
                            {
                                continue;
                            }
                            else
                            {
                                if (NPU_request_list[i][j][count].B <= NPU_size[i][j][t])
                                {
                                    temp_flag = 1;
                                    NPU_request_list[i][j][count].flag = 1;
                                    int end_time = t + process_time_calculate(i, NPU_request_list[i][j][count].B);
                                    for (int count2 = t; count2 < end_time; count2++)
                                    {
                                        NPU_size[i][j][count2] -= NPU_request_list[i][j][count].B;
                                    }
                                    user[NPU_request_list[i][j][count].user].end_time = max(user[NPU_request_list[i][j][count].user].end_time, end_time);
                                }
                            }
                        }
                    }
                }

            }
        }
    }
    /*for(int i=1;i<=M;i++)
    {
        cout<<"user_"<<i<<"_end_time:"<<user[i].end_time<<endl;
    }*/
    return;
}

double score_calculate()
{
    double score_sum = 0.0;
    double user_count = 0;
    for (int i = 1; i <= M; i++)
    {
        if (user[i].end_time > user[i].e)
        {
            user_count++;
        }
        double temp_x = double(user[i].end_time - user[i].e) / double(user[i].e - user[i].s);
        /*cout<<"temp_x:"<<temp_x<<endl;*/
        score_sum += h_x(temp_x) * p_x(user[i].move) * 10000.0;
        /*cout<<"score_temp:"<<fixed<<setprecision(6)<<h_x(temp_x) * p_x(user[i].move) * 10000.0<<endl;*/
    }
    /*cout<<"user_count:"<<user_count<<endl;*/
    score_sum = h_x(user_count) * score_sum;
    return score_sum;
}

int main()
{
    data_loader_generator(1);

    ifstream in("not_full_NPU.txt");
    for (int i = 1; i <= M; i++)
    {
        int Ti;
        in >> Ti;
        int last_server = -1;
        int last_NPU = -1;
        for (int j = 1; j <= Ti; j++)
        {
            Plan plan;
            in >> plan.timej >> plan.serverj >> plan.NPUj >> plan.Bj;
            solution[i].push_back(plan);
            if (last_NPU != -1 and last_server != -1 and (last_server != plan.serverj or last_NPU != plan.NPUj))
            {
                user[j].move++;
            }
            append_request(i, plan.timej , plan.serverj , plan.NPUj , plan.Bj);
            last_server = plan.serverj;
            last_NPU = plan.NPUj;
        }
    }
    in.close();
    brief_check();
    NPU_request_process();
    double score = score_calculate();
    if (score != -1)
    {
        cout << "The score you get of this sample is " << fixed << setprecision(0) << score << endl;
    }
    return 0;
}
