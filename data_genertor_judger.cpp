#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <algorithm>
using namespace std;
int N, g[11], k[11], m[11], M, latency[11][501], a, b, NPU_size[11][11][200001], request_size[11];
struct User
{
    int id, s, e, cnt;
    User() : id(0), s(0), e(0), cnt(0) {}
} user[501];

struct Plan
{
    int timej, serverj, NPUj, Bj, time_cost;
};
vector<Plan> solution[501];

void data_loader_generator(bool New)
{
    system("g++ main.cpp -lm -Wl,--stack=134217728 -O0 -std=c++11 -static-libstdc++ -static-libgcc -o main");
    if (!New)
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
            for (int j = 1; j <= g[i]; j++) // 初始化
                for (int k = 0; k <= 200000; k++)
                    NPU_size[i][j][k] = (m[i] - b) / a; // 确实应该向下取整，

        for (int i = 1; i <= N; i++)
        {
            request_size[i] = min(NPU_size[i][1][1], 1000); // 表示应该向第i个服务器的NPU放入多大的样本数量
        }
        system("main.exe < .\\sample\\data.in > output.txt");
    }

    srand(1); // 固定随机种子

    // 生成服务器种类 N ∈ [1, 10]
    N = rand() % 10 + 1;
    for (int i = 1; i <= N; ++i)
    {
        g[i] = rand() % 10 + 1;      // 1..10
        k[i] = rand() % 5 + 1;       // 1..5
        m[i] = rand() % 1001 + 1000; // 1000..2000
    }

    // 生成用户数量 M ∈ [1, 500]
    M = rand() % 500 + 1;

    // 为每个用户生成 s, e, cnt
    for (int i = 1; i <= M; ++i)
    {
        user[i].id = i;
        // cnt ∈ [1, 6000]
        user[i].cnt = rand() % 6000 + 1;
        int maxS = 60000 - user[i].cnt * 5;
        user[i].s = rand() % (maxS + 1);
        user[i].e = user[i].s + 5 * user[i].cnt + rand() % (60000 - user[i].s - 5 * user[i].cnt + 1);
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
    a = rand() % 11 + 10;   // 10..20
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

    ofs.close();
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
    cout << "Success";
}

void calulate_score()
{
    
}

int main()
{
    data_loader_generator(0);

    ifstream in("output.txt");
    for (int i = 1; i <= M; i++)
    {
        int Ti;
        in >> Ti;
        for (int j = 1; j <= Ti; j++)
        {
            Plan plan;
            in >> plan.timej >> plan.serverj >> plan.NPUj >> plan.Bj;
            solution[i].push_back(plan);
        }
    }
    in.close();

    // brief_check();

    calulate_score();
    return 0;
}
// 洗衣服