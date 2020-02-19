#ifndef SMOOTHLY_BUILDINGGRAPH
#define SMOOTHLY_BUILDINGGRAPH

#include <string>
#include <set>
#include <map>
#include <list>
#include <unordered_map>
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <functional>

#include <string.h>

#include <sqlite3.h>

#include "../utils/mempool.h"
#include "../utils/ipair.h"
#include "../utils/cyqueue.h"
#include "../utils/uuid.h"

namespace smoothly{

class buildingGraph{
    private:
        struct node{
                int             *   threadFlag;         //遍历标识符，不同线程各自使用
                std::set<node*>     connection;         //连接的节点
                buildingGraph   *   parent;
                bool                onFloor;            //位于地面
                std::string         uuid;
                node            *   next;

                void disconnect(node*);                 //和节点断开连接
                void disconnect_all();                  //和周围所有节点断开连接
                void remove();                          //删除（回收）节点
                void connect(node *, bool sync=true);   //连接
                void setOnFloor(bool f);                //设置接地
                node();
                ~node();
        };

        node * putNode(const std::string & ,bool sync=true);
        node * putNode(bool sync=true);
        node * createNode();

        mempool<node>                           nodePool;//node的内存池

        int                                 *   timeStep;//时间标志，不同线程各自使用
        std::unordered_map<std::string,node*>   nodes;
        circlequeue<node*>                  *   bfsQueue;

        std::atomic<bool>                       running;
        std::atomic<int>                        running_thread;

        std::mutex solvingMtx;      //求解前先上锁

        //外部通知主求解线程开始求解
        std::mutex bfs_solve_mtx;
        std::condition_variable bfs_solve_cv;
        void bfs_solve_wait();
        void bfs_solve_wake();

        //通知保存
        std::mutex bfs_save_mtx;
        std::condition_variable bfs_save_cv;
        void bfs_save_wait();
        void bfs_save_wake();

        std::queue<std::pair<std::string,std::list<std::string> > > requestToConnect;
        std::mutex                                                  requestToConnect_mtx;

        std::queue<std::pair<std::string,bool> >    requestToSetFloor;
        std::mutex                                  requestToSetFloor_mtx;

        std::queue<std::string> requestToRemove;
        std::mutex              requestToRemove_mtx;

        //主求解线程通知子求解线程开始求解
        std::mutex bfs_begin_mtx;
        std::condition_variable bfs_begin_cv;
        void bfs_begin_wait();
        void bfs_begin_wake();

        std::queue<std::string> waitForRemove;
        std::mutex waitForRemove_mtx;
        //外部发起删除请求后，先锁住mtx，然后插入uuid。
        //主求解线程先锁住solvingMtx，然后检测waitForRemove是否为空
        //不为空则令solvingNum为待求解的数量
        //然后bfs_begin_wake唤醒子线程求解
        //子线程每完成一个求解，--solvingNum并bfs_end_wake
        //主线程bfs_end_wait等待solvingNum为0，如不为0，bfs_begin_wake继续通知子线程求解
        //最后将判为删除的节点发送给外部，解锁

        //子求解线程通知主求解线程求解完成
        std::mutex bfs_end_mtx;
        std::condition_variable bfs_end_cv;
        void bfs_end_wait();
        void bfs_end_wake();

        std::atomic<int> solvingNum;//正在求解的线程数

        void solve_mainThread();
        void solve_subThread(int id);

        //算法：
        //发生删除事件后，先将要被删除的节点标记，然后从所有与其连接的节点开始广搜，如能发现onFloor的节点，直接停止该线程以节省资源
        //如能发现的节点数在bfs_limit以内，且其中无onFloor节点，则将这些节点也删除
        node                                *** bfs_reault;         //广搜结果（二维数组，第一维为线程id，第二维长度bfs_limit）
        std::set<node*>                     *   bfs_waitForSearch;  //等待搜索
        std::vector<std::string>                bfs_waitForRemove;
        std::mutex                              bfs_waitForRemove_mutex;
        bool                                *   bfs_onFloor;        //在地面上
        int                                     bfs_limit;          //广搜次数上限，超过直接throw
        void    BFSStart(node * m,int thread);
        int     markAndBFS(node * n,node * m, int thread);          //先标记m，再从n开始广搜
        //使用前先++timeStep[thread]
        //n:起始节点
        //m:发生删除事件的节点
        //thread:线程id
        //返回：被搜的节点个数

        void mainThread();
        void subThread(int id);

        std::mutex                  removeResult_mtx;
        std::queue<std::string>     removeResult;

        int threadNum;

        sqlite3         *   db;                 //数据库
        sqlite3_stmt    *   stmt_insert_node;   //插入节点
        sqlite3_stmt    *   stmt_insert_connect;//插入连接
        sqlite3_stmt    *   stmt_delete_node;   //删除节点
        sqlite3_stmt    *   stmt_delete_connect;//删除连接
        sqlite3_stmt    *   stmt_setFloor;      //设置接地
        void db_insert_node(const std::string & uuid);
        void db_insert_connection(const std::string & from , const std::string & to);
        void db_delete_node(const std::string & uuid);
        void db_delete_connection(const std::string & from , const std::string & to);
        void db_setFloor(const std::string & uuid , bool f);
        inline void db_begin(){
            sqlite3_exec(db,"begin;",0,0,0);
        }
        inline void db_commit(){
            sqlite3_exec(db,"commit;",0,0,0);
        }

        struct db_cmd{
                enum method_t{
                    ADD_NODE,
                    ADD_CONN,
                    DEL_NODE,
                    DEL_CONN,
                    SET_FLOOR
                };
                method_t method;
                std::string str1,str2;
                bool setfloor;
                inline db_cmd():str1(),str2(){}
                inline db_cmd(const db_cmd & i):str1(i.str1),str2(i.str2),setfloor(i.setfloor){
                    method = i.method;
                }
                inline db_cmd(method_t m,const std::string & s1,bool f):str1(s1){
                    method      = m;
                    setfloor    = f;
                }
                inline db_cmd(method_t m,const std::string & s1,const std::string & s2):str1(s1),str2(s2){
                    method = m;
                }
        };
        std::mutex          db_cmd_queue_mtx;
        std::queue<db_cmd>  db_cmd_queue;
        void db_doCmd(const db_cmd & cmd);
        void db_write(const buildingGraph::db_cmd & cmd);
        void db_flush();


    public:
        buildingGraph(const std::string & db , int threadNum = 16, int limit = 128);
        ~buildingGraph();

        //请求删除
        inline void requestRemove(const std::string & r){
            requestToRemove_mtx.lock();
            requestToRemove.push(r);
            requestToRemove_mtx.unlock();
            bfs_solve_wake();//唤醒求解主线程
        }

        //请求连接（或添加）
        inline void requestAdd(const std::string & uuid,const std::list<std::string> & conn){
            requestToConnect_mtx.lock();
            requestToConnect.push(std::pair<std::string,std::list<std::string> >(uuid,conn));
            requestToConnect_mtx.unlock();
            bfs_solve_wake();
        }

        //请求设置为接地节点
        inline void requestSetFloor(const std::string & uuid,bool m){
            requestToSetFloor_mtx.lock();
            requestToSetFloor.push(std::pair<std::string,bool>(uuid,m));
            requestToSetFloor_mtx.unlock();
            bfs_solve_wake();
        }

        //获取被执行删除的节点
        inline void getRemoved(std::function<bool(const std::string & s)> callback){
            bool runn=true;
            removeResult_mtx.lock();
            while (!removeResult.empty() && runn) {
                std::string uuid = removeResult.front();
                removeResult.pop();
                removeResult_mtx.unlock();
                runn = callback(uuid);
                removeResult_mtx.lock();
            }
            removeResult_mtx.unlock();
        }
};

}
#endif // BUILDINGGRAPH_H
