#include "dig.h"
#include <QByteArray>

namespace smoothly{
namespace server{

dig::dig(){
    mkdir("./map",0777);
    mkdir("./map/digmap",0777);
    //std::vector<std::pair<std::pair<int32_t, int32_t>, int16_t> > d;
    //d.push_back(std::pair<std::pair<int32_t, int32_t>, int16_t>(std::pair<int32_t, int32_t>(-2218,3716),-10));
    //setDigDepth(d);
}

void dig::setDigDepth(const std::vector<std::pair<std::pair<int32_t, int32_t>, int16_t> > & dig){
    for(auto it:dig){
        int x = it.first.first;
        int y = it.first.second;
        int16_t d = it.second;

        //chunk
        int cx = floor(x/32.0);
        int cy = floor(y/32.0);

        //边缘的特殊处理
        int xedge = x%32;
        int yedge = y%32;

        if(xedge==0){
            //x边缘
            if(yedge==0){
                //x y 边缘
                int xBegin_xy = (cx-1)*32;
                int yBegin_xy = (cy-1)*32;
                setDigInChunk(cx-1,cy-1,x-xBegin_xy,y-yBegin_xy,d);
                //处理y边缘
                int xBegin_y = cx*32;
                int yBegin_y = (cy-1)*32;
                setDigInChunk(cx,cy-1,x-xBegin_y,y-yBegin_y,d);
            }

            int xBegin_x = (cx-1)*32;
            int yBegin_x = cy*32;
            setDigInChunk(cx-1,cy,x-xBegin_x,y-yBegin_x,d);

        }else{
            if(yedge==0){
                //y边缘
                int xBegin_y = cx*32;
                int yBegin_y = (cy-1)*32;
                setDigInChunk(cx,cy-1,x-xBegin_y,y-yBegin_y,d);
            }
        }

        //本区块的
        int xBegin = cx*32;
        int yBegin = cy*32;
        setDigInChunk(cx,cy,x-xBegin,y-yBegin,d);
    }
    //构建缓存
    buildDigCache();
}

void dig::sendDigMap(const RakNet::SystemAddress & addr, int32_t x, int32_t y){
    char key[256];
    snprintf(key,sizeof(key),"digmap:%d,%d" , x , y);
    std::string value;
    if(db->Get(leveldb::ReadOptions(), key , &value).ok() && !value.empty()){
        auto v = QByteArray::fromBase64(value.c_str());
        sendAddr_digMap(addr , x , y , v.data() , v.length());
    }
}

void dig::setDigInChunk(int x, int y, int rx, int ry, int16_t depth){
    setDigInChunk(x,y,rx+ry*33,depth);
}

void dig::setDigInChunk(int x, int y, int index, int16_t depth){
    if(index<0 || index>=33*33)
        return;

    auto it = mapHandlers.find(ipair(x,y));
    if(it==mapHandlers.end()){
        int fd = 0;
        int16_t *ptr = NULL;
        struct stat st;

        //路径
        char path[128];
        snprintf(path,sizeof(path),"./map/digmap/%d-%d.cm",x,y);

        //大小
        static const int size = 33*33*sizeof(int16_t);

        if((fd = open(path, O_RDWR|O_CREAT,0777)) < 0){
            printf("open file error\n");
            return;
        }

        //获取大小
        if(fstat(fd, &st) < 0){
            printf("get file state error:%d\n", errno);
            close(fd);
            return;
        }

        //改变大小
        if(st.st_size!=size){
            if(ftruncate(fd,size)<0){
                printf("fail to truncate\n");
                close(fd);
                return;
            }
        }

        //开始映射
        ptr = (int16_t*)mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
        if (ptr == MAP_FAILED){
            printf("mmap failed\n");
            close(fd);
            return;
        }

        ptr[index] = depth;
        auto buf = std::tuple<int,int16_t*,std::vector<std::pair<int16_t,int16_t> > >(
                    fd ,
                    ptr ,
                    std::vector<std::pair<int16_t,int16_t> >());
        std::get<2>(buf).push_back(std::pair<int16_t,int16_t>(index,depth));

        mapHandlers[ipair(x,y)] = std::move(buf);
    }else{
        std::get<1>(it->second)[index] = depth;
        std::get<2>(it->second).push_back(std::pair<int16_t,int16_t>(index,depth));
    }
}

void dig::buildDigCache(){
    if(mapHandlers.empty())
        return;

    static const int size = 33*33*sizeof(int16_t);
    char key[256];

    leveldb::WriteBatch batch;

    for(auto & dig:mapHandlers){
        RakNet::BitStream bs;
        bool bbuf;

        snprintf(key,sizeof(key),"digmap:%d,%d" , dig.first.x , dig.first.y);

        int16_t * ptr = std::get<1>(dig.second);
        int i = 0;
        uint16_t zeroCount = 0;

        //生成缓存
        while(i<33*33){
            if(ptr[i]==0){
                ++zeroCount;
            }else{
                //清空压缩
                if(zeroCount>0){
                    bbuf = true;
                    bs.Write(bbuf);
                    bs.Write(zeroCount);
                    zeroCount = 0;
                }
                //写入数据
                bbuf = false;
                bs.Write(bbuf);
                bs.Write((int16_t)ptr[i]);
                zeroCount = 0;
            }
            ++i;
        }

        //清空压缩
        if(zeroCount>0){
            bbuf = true;
            bs.Write(bbuf);
            bs.Write(zeroCount);
            zeroCount = 0;
        }

        batch.Put(key,QByteArray((const char *)bs.GetData(),bs.GetNumberOfBytesUsed()).toBase64().toStdString());

        munmap(ptr, size);
        close(std::get<0>(dig.second));

        boardcast_setDigDepth(dig.first.x , dig.first.y , std::get<2>(dig.second));
    }

    db->Write(leveldb::WriteOptions(), &batch);
    mapHandlers.clear();
}

}//////server
}//////smoothly
