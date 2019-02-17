#ifndef SMOOTHLY_BUILDING
#define SMOOTHLY_BUILDING
#include "terrain.h"
#include "hbb.h"
#include "remoteGraph.h"
namespace smoothly{
    class buildings:public terrain,public remoteGraph{
        public:
            
            virtual void onGenChunk(terrain::chunk *);
            virtual void onFreeChunk(terrain::chunk *);
            virtual void onGenBuilding(remoteGraph::item *);
            virtual void onFreeBuilding(remoteGraph::item *);
            virtual void onDestroyBuilding(remoteGraph::item *);
            virtual void onCreateBuilding(remoteGraph::item *);
    };
}
#endif