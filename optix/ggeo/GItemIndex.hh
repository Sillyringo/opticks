#pragma once

class GColors ; 
class GColorMap ; 
class GBuffer ; 

struct gfloat3 ; 
class Index ; 
class Types ; 

#include "string.h"
#include <string>
#include <map>
#include <vector>

//
// TODO: sort out tension/duplication between GItemIndex and Index
//   NB indices are used in a variety of circumstances, so this aint trivial
//
//   * materials
//   * flags
//   * material sequences
//   * flag sequences
//
//
// adds colors and gui to basis Index constituent 
class GItemIndex {
   public:
        GItemIndex(Index* index);
        GItemIndex(const char* itemtype);
        void setTitle(const char* title);
   public:
        typedef std::string (*GItemIndexLabellerPtr)(GItemIndex*, const char*, unsigned int& );
        typedef enum { DEFAULT, COLORKEY, MATERIALSEQ, HISTORYSEQ } Labeller_t ;
   public:
        static std::string defaultLabeller(GItemIndex* self, const char* key, unsigned int& colorcode);
        static std::string colorKeyLabeller(GItemIndex* self, const char* key, unsigned int& colorcode);
        static std::string materialSeqLabeller(GItemIndex* self, const char* key, unsigned int& colorcode);
        static std::string historySeqLabeller(GItemIndex* self, const char* key, unsigned int& colorcode);
   public:
        void setLabeller(GItemIndexLabellerPtr labeller);
        void setLabeller(Labeller_t labeller );
        std::string getLabel(const char* key, unsigned int& colorcode);
   public:
        void     setTypes(Types* types);
        Types*   getTypes();
   public:
        void add(const char* name, unsigned int source);
   public:
        bool         hasItem(const char* name);
        unsigned int getIndexLocal(const char* name, unsigned int missing=0);
        unsigned int getNumItems();
        unsigned int getColorCode(const char* key);
        const char*  getColorName(const char* key);
        static gfloat3* makeColor( unsigned int rgb );

   public:
        Index*       getIndex();
        int          getSelected();
   public:
        void loadIndex(const char* idpath, const char* override=NULL);
   private:
        void init(const char* itemtype);
   public:
        static GItemIndex* load(const char* idpath, const char* itemtype);
        void save(const char* idpath);
        void dump(const char* msg="GItemIndex::dump");
        void test(const char* msg="GItemIndex::test", bool verbose=true);

   public:
        // color
        void     setColorSource(GColors* colors);
        void     setColorMap(GColorMap* colormap);

        GColors* getColorSource();
        GColorMap* getColorMap();

        GBuffer* makeColorBuffer();
        GBuffer* getColorBuffer();

        std::vector<unsigned int>&    getCodes();
        std::vector<std::string>&     getLabels();

   public:
        void     formTable(bool verbose=false);

   private:
        Index*                               m_index ; 
        GItemIndexLabellerPtr                m_labeller ;
   private:
        GColors*                             m_colors ; 
        GColorMap*                           m_colormap ; 
        GBuffer*                             m_colorbuffer ; 
        Types*                               m_types ; 
   private:
        // populated by formTable
        std::vector<std::string>             m_labels ; 
        std::vector<unsigned int>            m_codes ; 
};

inline GItemIndex::GItemIndex(const char* itemtype)
   : 
   m_index(NULL),
   m_colors(NULL),
   m_colormap(NULL),
   m_colorbuffer(NULL),
   m_types(NULL)
{
   init(itemtype);
   setLabeller(DEFAULT);
}



inline GItemIndex::GItemIndex(Index* index)
   : 
   m_index(index),
   m_colors(NULL),
   m_colormap(NULL),
   m_colorbuffer(NULL),
   m_types(NULL)
{
   setLabeller(DEFAULT);
}


inline void GItemIndex::setLabeller(GItemIndexLabellerPtr labeller)
{
   m_labeller = labeller ; 
}
inline void GItemIndex::setColorSource(GColors* colors)
{
   m_colors = colors ; 
}
inline void GItemIndex::setColorMap(GColorMap* colormap)
{
   m_colormap = colormap ; 
}
inline void GItemIndex::setTypes(Types* types)
{
   m_types = types ; 
}
inline Types* GItemIndex::getTypes()
{
   return m_types ;
}


inline GColors* GItemIndex::getColorSource()
{
   return m_colors ; 
}
inline GColorMap* GItemIndex::getColorMap()
{
   return m_colormap ; 
}

inline Index* GItemIndex::getIndex()
{
   return m_index ; 
}

inline std::vector<unsigned int>& GItemIndex::getCodes()
{
   return m_codes ; 
}

inline std::vector<std::string>& GItemIndex::getLabels()
{
   return m_labels ; 
}



