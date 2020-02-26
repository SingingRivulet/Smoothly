#include "buildinggraph.h"
#include "fontcolor.h"

namespace smoothly{

buildingGraph::node *buildingGraph::putNode(const std::string & uuid, bool sync)
{
    auto rit = nodes.find(uuid);
    if(rit!=nodes.end()){
        return rit->second;
    }else{
        auto n      = createNode();
        n->uuid     = uuid;
        nodes[uuid] = n;
        if(sync){
            db_write(db_cmd(db_cmd::ADD_NODE , uuid , ""));
        }
        return n;
    }
}

buildingGraph::node *buildingGraph::putNode(bool sync)
{
    std::string uuid;
    do{
        getUUID(uuid);
    }while(nodes.find(uuid)!=nodes.end());
    auto n      = createNode();
    n->uuid     = uuid;
    nodes[uuid] = n;
    if(sync){
        db_write(db_cmd(db_cmd::ADD_NODE , n->uuid , ""));
    }
    return n;
}

buildingGraph::node *buildingGraph::createNode()
{
    auto n    = nodePool.get();
    n->parent = this;
    n->connection.clear();
    if(n->threadFlag==NULL){
        n->threadFlag = new int[threadNum];
    }
    n->onFloor= false;
    return n;
}

void buildingGraph::bfs_getResult_wait()
{
    std::unique_lock<std::mutex> lck(bfs_getResult_mtx);
    bfs_getResult_cv.wait(lck);
}

void buildingGraph::bfs_getResult_wake()
{
    std::unique_lock<std::mutex> lck(bfs_getResult_mtx);
    bfs_getResult_cv.notify_all();
}

void buildingGraph::bfs_solve_wait()
{
    std::unique_lock<std::mutex> lck(bfs_solve_mtx);
    bfs_solve_cv.wait(lck);
}

void buildingGraph::bfs_solve_wake()
{
    std::unique_lock<std::mutex> lck(bfs_solve_mtx);
    bfs_solve_cv.notify_all();
}

void buildingGraph::bfs_save_wait()
{
    std::unique_lock<std::mutex> lck(bfs_save_mtx);
    bfs_save_cv.wait(lck);
}

void buildingGraph::bfs_save_wake()
{
    std::unique_lock<std::mutex> lck(bfs_save_mtx);
    bfs_save_cv.notify_all();
}

void buildingGraph::bfs_begin_wait()
{
    std::unique_lock<std::mutex> lck(bfs_begin_mtx);
    bfs_begin_cv.wait(lck);
}

void buildingGraph::bfs_begin_wake()
{
    std::unique_lock<std::mutex> lck(bfs_begin_mtx);
    bfs_begin_cv.notify_all();
}

void buildingGraph::bfs_end_wait()
{
    std::unique_lock<std::mutex> lck(bfs_end_mtx);
    bfs_end_cv.wait(lck);
}

void buildingGraph::bfs_end_wake()
{
    std::unique_lock<std::mutex> lck(bfs_end_mtx);
    bfs_end_cv.notify_all();
}

void buildingGraph::solve_mainThread()
{
    solvingMtx.lock();

    requestToConnect_mtx.lock();
    while (!requestToConnect.empty()) { //连接节点
        auto p = requestToConnect.front();
        auto n = putNode(p.first);      //找到节点
        for(auto it:p.second){          //建立连接
            auto rit = nodes.find(it);  //定位被连接的节点
            if(rit!=nodes.end()){
                n->connect(rit->second);
            }
        }
        requestToConnect.pop();
    }
    requestToConnect_mtx.unlock();

    requestToSetFloor_mtx.lock();
    while(!requestToSetFloor.empty()){
        auto p = requestToSetFloor.front();
        auto n = putNode(p.first);
        n->setOnFloor(p.second);
        requestToSetFloor.pop();
    }
    requestToSetFloor_mtx.unlock();

    waitForRemove_mtx.lock();
    requestToRemove_mtx.lock();
    while (!requestToRemove.empty()) {
        waitForRemove.push(requestToRemove.front());
        requestToRemove.pop();
    }
    requestToRemove_mtx.unlock();

    if(!waitForRemove.empty()){
        solvingNum = waitForRemove.size();
        waitForRemove_mtx.unlock();

        while(solvingNum!=0){
            bfs_begin_wake();
            bfs_end_wait();
        }

        //最后执行删除并向外部发送
        //待删除的都在bfs_waitForRemove
        bfs_waitForRemove_mutex.lock();
        //先上锁，然后逐个读取删除并发送
        removeResult_mtx.lock();
        for(auto it:bfs_waitForRemove){
            auto rit = nodes.find(it);
            if(rit!=nodes.end()){
                removeResult.push(it);//装入结果集
                rit->second->remove();//删除
            }
        }
        removeResult_mtx.unlock();
        bfs_waitForRemove.clear();
        bfs_waitForRemove_mutex.unlock();
        bfs_getResult_wake();

    }else{
        waitForRemove_mtx.unlock();
    }

    solvingMtx.unlock();
}

void buildingGraph::solve_subThread(int id)
{
    //bfs_begin_wait();
    //此函数由上一个函数调用
    //因为还要处理程序退出
    std::string uuid;
    waitForRemove_mtx.lock();
    if(!waitForRemove.empty()){
        //读取队列
        uuid = waitForRemove.front();
        waitForRemove.pop();
    }else{
        //空队列，唤醒主线程然后结束
        waitForRemove_mtx.unlock();
        bfs_end_wake();
        return;
    }
    waitForRemove_mtx.unlock();

    //uuid拿到了，开始求解
    auto rit = nodes.find(uuid);
    if(rit!=nodes.end()){
        BFSStart(rit->second,id);
    }

    //求解完唤醒主线程
    --solvingNum;
    bfs_end_wake();
}

void buildingGraph::BFSStart(buildingGraph::node * m, int thread)
{
    bfs_waitForSearch[thread]=m->connection;

    bfs_waitForRemove_mutex.lock();
    bfs_waitForRemove.push_back(m->uuid);
    bfs_waitForRemove_mutex.unlock();

    while(!bfs_waitForSearch[thread].empty()){
        auto it = bfs_waitForSearch[thread].begin();

        ++(timeStep[thread]);
        try{
            int len = markAndBFS(*it , m , thread);
            if(!bfs_onFloor[thread]){
                //执行删除bfs_reault[thread]
                for(int i=0;i<len;i++){
                    bfs_waitForRemove_mutex.lock();
                    bfs_waitForRemove.push_back(bfs_reault[thread][i]->uuid);
                    bfs_waitForRemove_mutex.unlock();
                }
            }
        }catch(...){
            //不执行删除
        }

        bfs_waitForSearch[thread].erase(it);
    }


}

int buildingGraph::markAndBFS(buildingGraph::node * n, buildingGraph::node * m, int thread)
{
    m->threadFlag[thread] = timeStep[thread];
    int count = 0;
    bfs_onFloor[thread]=false;

    circlequeue<node*> & bq = bfsQueue[thread];
    bq.push(n);

    while(!bq.isEmpty()){

        node * tn = bq.top();//获取队首
        bq.pop();//出队
        bfs_reault[thread][count] = tn;//加入结果集
        ++count;

        if(count>=bfs_limit){
            throw std::out_of_range("节点数量超限");
        }

        if(tn->connection.find(m)!=tn->connection.end() && tn!=n){//与事件节点相连且不是起始节点
            bfs_waitForSearch[thread].erase(tn);//下一次不再搜索这个节点
        }

        if(tn->onFloor){
            bfs_onFloor[thread]=true;
        }

        tn->threadFlag[thread] = timeStep[thread];//标记
        for(auto it:tn->connection){//遍历相连的节点
            if(it->threadFlag[thread] != timeStep[thread]){//未被标记，入队
                bq.push(it);
            }
        }

    }
    return count;
}

void buildingGraph::mainThread()
{
    bfs_solve_wait();
    while(running){
        solve_mainThread();
        bfs_solve_wait();
        if(!running)
            break;
    }
    --running_thread;
    bfs_end_wake();
}

void buildingGraph::subThread(int id)
{
    bfs_begin_wait();
    while(running){
        solve_subThread(id);
        bfs_begin_wait();
        if(!running)
            break;
    }
    --running_thread;
    bfs_end_wake();
}

void buildingGraph::db_insert_node(const std::string & uuid)
{
    sqlite3_reset(stmt_insert_node);
    sqlite3_bind_text(stmt_insert_node, 1, uuid.c_str(), uuid.size(), SQLITE_STATIC);
    sqlite3_step(stmt_insert_node);
}

void buildingGraph::db_insert_connection(const std::string & from, const std::string & to)
{
    sqlite3_reset(stmt_insert_connect);
    sqlite3_bind_text(stmt_insert_connect, 1, from.c_str(), from.size(), SQLITE_STATIC);
    sqlite3_bind_text(stmt_insert_connect, 2, to.c_str(), to.size(), SQLITE_STATIC);
    sqlite3_step(stmt_insert_connect);
}

void buildingGraph::db_delete_node(const std::string & uuid)
{
    sqlite3_reset(stmt_delete_node);
    sqlite3_bind_text(stmt_delete_node, 1, uuid.c_str(), uuid.size(), SQLITE_STATIC);
    sqlite3_step(stmt_delete_node);
}

void buildingGraph::db_delete_connection(const std::string & from, const std::string & to)
{
    sqlite3_reset(stmt_delete_connect);
    sqlite3_bind_text(stmt_delete_connect, 1, from.c_str(), from.size(), SQLITE_STATIC);
    sqlite3_bind_text(stmt_delete_connect, 2, to.c_str(), to.size(), SQLITE_STATIC);
    sqlite3_step(stmt_delete_connect);

    sqlite3_reset(stmt_delete_connect);
    sqlite3_bind_text(stmt_delete_connect, 1, to.c_str(), to.size(), SQLITE_STATIC);
    sqlite3_bind_text(stmt_delete_connect, 2, from.c_str(), from.size(), SQLITE_STATIC);
    sqlite3_step(stmt_delete_connect);
}

void buildingGraph::db_delete_all_connection(const std::string & from)
{
    sqlite3_reset(stmt_delete_all_connect);
    sqlite3_bind_text(stmt_delete_all_connect, 1, from.c_str(), from.size(), SQLITE_STATIC);
    sqlite3_bind_text(stmt_delete_all_connect, 2, from.c_str(), from.size(), SQLITE_STATIC);
    sqlite3_step(stmt_delete_all_connect);
}

void buildingGraph::db_setFloor(const std::string & uuid, bool f)
{
    sqlite3_reset(stmt_setFloor);
    sqlite3_bind_int(stmt_setFloor, 1, (f ? 1 : 0));
    sqlite3_bind_text(stmt_setFloor, 2, uuid.c_str(), uuid.size(), SQLITE_STATIC);
    sqlite3_step(stmt_setFloor);
}

void buildingGraph::db_doCmd(const buildingGraph::db_cmd & cmd)
{
    db_begin();
    if(cmd.method==db_cmd::ADD_CONN){
        db_insert_connection(cmd.str1 , cmd.str2);
    }else if(cmd.method==db_cmd::ADD_NODE){
        db_insert_node(cmd.str1);
    }else if(cmd.method==db_cmd::DEL_CONN){
        db_delete_connection(cmd.str1 , cmd.str2);
    }else if(cmd.method==db_cmd::DEL_NODE){
        db_delete_node(cmd.str1);
    }else if(cmd.method==db_cmd::SET_FLOOR){
        db_setFloor(cmd.str1,cmd.setfloor);
    }else if(cmd.method==db_cmd::DEL_ALL_CONN){
        db_delete_all_connection(cmd.str1);
    }
    db_commit();
}

void buildingGraph::db_write(const buildingGraph::db_cmd & cmd)
{
    db_cmd_queue_mtx.lock();
    db_cmd_queue.push(cmd);
    db_cmd_queue_mtx.unlock();
    bfs_save_wake();
}

void buildingGraph::db_flush()
{
    bfs_save_wait();
    while(running){

        db_cmd_queue_mtx.lock();
        while(!db_cmd_queue.empty()){
            db_cmd c = db_cmd_queue.front();
            db_cmd_queue.pop();
            db_cmd_queue_mtx.unlock();
            db_doCmd(c);
            db_cmd_queue_mtx.lock();
        }
        db_cmd_queue_mtx.unlock();

        bfs_save_wait();
        if(!running)
            break;
    }
    --running_thread;
    bfs_end_wake();
}

buildingGraph::buildingGraph(const std::string & dbpath, int threadNum, int limit)
{
    running = true;
    running_thread=2;

    bfs_limit           = limit;
    this->threadNum     = threadNum;

    //初始化缓冲区
    bfs_reault          = new node**            [threadNum];
    bfs_waitForSearch   = new std::set<node*>   [threadNum];
    bfs_onFloor         = new bool              [threadNum];
    timeStep            = new int               [threadNum];
    bfsQueue            = new circlequeue<node*>[threadNum];
    for(int i=0;i<threadNum;++i){
        bfs_reault  [i] = new node*[limit];
        timeStep    [i] = 0;
    }

    bool instal = false;
    {
        auto fp = fopen(dbpath.c_str() , "r");
        if(fp==NULL)
            instal = true;
        else
            fclose(fp);
    }
    //启动数据库
    if(sqlite3_open(dbpath.c_str(), &db)){
        printf(L_RED "[error]" NONE "buildingSolver:Can't open database: %s\n", sqlite3_errmsg(db));
    }else{
        printf(L_GREEN "[status]" NONE "buildingSolver:Opened database successfully\n");

        if(instal){
            printf(L_GREEN "[status]" NONE "buildingSolver:instal database\n");
            //安装
            sqlite3_exec(db,"create table node([uuid] char(64),[onfloor] int default 0)",0,0,0);
            sqlite3_exec(db,"create table connection([node_from] char(64),[node_to] char(64))",0,0,0);
            //创建索引
            sqlite3_exec(db,"create unique index node_unique on node ([uuid])",0,0,0);
            sqlite3_exec(db,"create unique index connection_index on connection ([node_from],[node_to])",0,0,0);
            sqlite3_exec(db,"create unique index connection_index_to on connection ([node_to])",0,0,0);
        }else{
            //读取
        }

        //编译sql语句
        const static char sql_insert_node[] = "insert into node (uuid)values(?)";
        if(sqlite3_prepare_v2(db,sql_insert_node,strlen(sql_insert_node),&stmt_insert_node,0)==SQLITE_OK){
            printf(L_GREEN "[status]" NONE "buildingSolver:compile sql:1 success\n");
        }else{
            printf(L_RED "[error]" NONE "buildingSolver:compile sql:1 fail\n");
        }

        const static char sql_insert_connect[] = "insert into connection values(?,?)";
        if(sqlite3_prepare_v2(db,sql_insert_connect,strlen(sql_insert_connect),&stmt_insert_connect,0)==SQLITE_OK){
            printf(L_GREEN "[status]" NONE "buildingSolver:compile sql:2 success\n");
        }else{
            printf(L_RED "[error]" NONE "buildingSolver:compile sql:2 fail\n");
        }

        const static char sql_delete_node[] = "delete from node indexed by node_unique where uuid=?";
        if(sqlite3_prepare_v2(db,sql_delete_node,strlen(sql_delete_node),&stmt_delete_node,0)==SQLITE_OK){
            printf(L_GREEN "[status]" NONE "buildingSolver:compile sql:3 success\n");
        }else{
            printf(L_RED "[error]" NONE "buildingSolver:compile sql:3 fail\n");
        }

        const static char sql_delete_connect[] = "delete from connection indexed by connection_index where node_from=? and node_to=?";
        if(sqlite3_prepare_v2(db,sql_delete_connect,strlen(sql_delete_connect),&stmt_delete_connect,0)==SQLITE_OK){
            printf(L_GREEN "[status]" NONE "buildingSolver:compile sql:4 success\n");
        }else{
            printf(L_RED "[error]" NONE "buildingSolver:compile sql:4 fail\n");
        }

        const static char sql_delete_all_connect[] = "delete from connection indexed by connection_index where node_from=?;"
                                                     "delete from connection indexed by connection_index_to where node_to=?";
        if(sqlite3_prepare_v2(db,sql_delete_all_connect,strlen(sql_delete_all_connect),&stmt_delete_all_connect,0)==SQLITE_OK){
            printf(L_GREEN "[status]" NONE "buildingSolver:compile sql:5 success\n");
        }else{
            printf(L_RED "[error]" NONE "buildingSolver:compile sql:5 fail\n");
        }

        const static char sql_setFloor[] = "update node indexed by node_unique set onfloor=? where uuid=?";
        if(sqlite3_prepare_v2(db,sql_setFloor,strlen(sql_setFloor),&stmt_setFloor,0)==SQLITE_OK){
            printf(L_GREEN "[status]" NONE "buildingSolver:compile sql:6 success\n");
        }else{
            printf(L_RED "[error]" NONE "buildingSolver:compile sql:6 fail\n");
        }

        //载入数据
        //node表
        char *zErrMsg = NULL;
        if( sqlite3_exec(
                db,
                "select * from node",
                [](void *data, int ncolumn, char **colVal, char **){
                    auto self = (buildingGraph*)data;
                    if(ncolumn>=2){
                        auto n = self->putNode(colVal[0],false);
                        if(strcmp(colVal[1],"1")==0){
                            n->onFloor = true;
                        }else{
                            n->onFloor = false;
                        }
                    }
                    return 0;
                },
                this,
                &zErrMsg
                ) != SQLITE_OK ){
            printf(L_RED "[error]" NONE "buildingSolver:SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        //connection表
        if( sqlite3_exec(
                db,
                "select * from connection",
                [](void *data, int ncolumn, char **colVal, char **){
                    auto self = (buildingGraph*)data;
                    if(ncolumn>=2){
                        self->putNode(colVal[0],false)->connect(self->putNode(colVal[1],false),false);
                    }
                    return 0;
                },
                this,
                &zErrMsg
                ) != SQLITE_OK ){
            printf(L_RED "[error]" NONE "buildingSolver:SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
    }

    std::atomic<int> started(2);
    //创建线程
    std::thread mt([&](buildingGraph * self){
        printf(L_GREEN "[status]" NONE "buildingSolver:create solving thread\n");
        --started;
        self->mainThread();
    },this);
    mt.detach();
    std::thread sv([&](buildingGraph * self){
        printf(L_GREEN "[status]" NONE "buildingSolver:create database manager thread\n");
        --started;
        self->db_flush();
    },this);
    sv.detach();
    for(int i=0;i<threadNum;++i){
        ++running_thread;
        ++started;
        std::thread st([&](buildingGraph * self,int id){
            printf(L_GREEN "[status]" NONE "buildingSolver:create subthread:%d\n",id);
            --started;
            self->subThread(id);
        },this,i);
        st.detach();
    }
    while (started>0) {
        RakSleep(30);
    }


    //test
    //std::list<std::string> buffer;
    //requestAdd("a",buffer);
    //buffer.push_back("a");
    //requestSetFloor("a",true);
    //requestAdd("b",buffer);
    //requestRemove("a");
    //waitForResult();
    //printf("solve building finished\n");
    //getRemoved([&](const std::string & u){
    //    printf("removed:%s\n",u.c_str());
    //    return true;
    //});
}

buildingGraph::~buildingGraph()
{
    for(auto it:nodes){
        nodePool.del(it.second);
    }
    nodes.clear();
    running = false;
    bfs_begin_wake();
    bfs_solve_wake();
    bfs_save_wake();
    while(running_thread>0)
        bfs_end_wait();

    for(int i=0;i<threadNum;++i){
        delete [] bfs_reault[i];
    }
    delete [] bfs_reault;
    delete [] bfs_waitForSearch;
    delete [] bfs_onFloor;
    delete [] timeStep;
    delete [] bfsQueue;

    if(db){
        sqlite3_finalize(stmt_insert_node);
        sqlite3_finalize(stmt_insert_connect);
        sqlite3_finalize(stmt_delete_node);
        sqlite3_finalize(stmt_delete_connect);
        sqlite3_finalize(stmt_delete_all_connect);
        sqlite3_finalize(stmt_setFloor);
        sqlite3_close(db);
    }
}

void buildingGraph::node::disconnect(buildingGraph::node * n)
{
    n->connection.erase(this);
    connection.erase(n);
}

void buildingGraph::node::disconnect_all()
{
    for(auto it:connection){
        it->connection.erase(this);
    }
    connection.clear();
}

void buildingGraph::node::remove()
{
    parent->db_write(db_cmd(db_cmd::DEL_NODE , uuid,""));
    parent->db_write(db_cmd(db_cmd::DEL_ALL_CONN , uuid,""));
    disconnect_all();
    parent->nodes.erase(uuid);
    parent->nodePool.del(this);
}

void buildingGraph::node::connect(buildingGraph::node * n,bool sync)
{
    auto it = connection.find(n);
    if(it==connection.end()){
        if(sync){
            parent->db_write(db_cmd(db_cmd::ADD_CONN , uuid , n->uuid));
        }
        n->connection.insert(this);
        connection.insert(n);
    }
}

void buildingGraph::node::setOnFloor(bool f)
{
    onFloor = f;
    parent->db_write(db_cmd(db_cmd::SET_FLOOR,uuid,f));
}

buildingGraph::node::node()
{
    threadFlag = NULL;
}

buildingGraph::node::~node()
{
    if(threadFlag)
        delete [] threadFlag;
}

}
