#ifndef CStructureH
#define CStructureH

#include "CStream.h"
//#include "CParSet.h"

#include <string>
#include <map>
#include <ostream>

class CStructure
{
    friend std::ostream& operator<<(std::ostream&, const CStructure&);
	friend std::istream& operator>>(std::istream& is, CStructure& s);

public:
    CStructure();

    unsigned int size(void) { return streams.size(); }
    void clear(void);

    void Reset(void);
    bool NextStream(void);
    std::istream& serialize(std::istream&);
    std::string tokenize(std::istream&, std::string);
 //   bool CheckIntegrity(CParSet*);
    bool FindReachWithInput(std::string);

    // Get something
    CStream& GetCurrentStream(void);
    CStream& GetStreamByID(const std::string&);
    const CReach& FindReachByID(const std::string&);
    const CStream& FindStreamByReachID(const std::string&);
    CReach GetReachWithInput(std::string);
    bool Changed(void) { return changed; }
    std::string Filename(void) { return filename; }
    std::vector<std::string> GetAllReachIDs(void);
    bool Generated(void) { return generated; }

    // Set something
    void AddStream(CStream&);
    void Name(const std::string& n) { name = n; }
    void SetDirection(unsigned int d) { direction = d; }
    void Changed(bool c) { changed = c; }
    void Filename(std::string f) { filename = f; }
    void Generated(bool g) { generated = g; }

    enum directions {MainStemFirst, MainStemLast};
    void writeXml ( tinyxml2::XMLNode * node ) const;
    void readXml( tinyxml2::XMLNode * group );

private:
    typedef std::map<std::string, CStream> streamType;
    typedef streamType::iterator streamIter;
    typedef streamType::const_iterator streamConstIter;

    typedef std::multimap<unsigned short, std::string> streamOrderType;
    typedef streamOrderType::iterator streamOrderIter;
    typedef streamOrderType::const_iterator streamOrderConstIter;

    std::string name;
    streamType streams;
    streamOrderType streamOrder;
    streamOrderIter currentStream;
    unsigned int streamCount;
    unsigned int direction;
    bool changed, generated;
    std::string filename;
};

#endif