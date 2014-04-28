#ifndef _SEARCH_STEMMER_H_
#define _SEARCH_STEMMER_H_

#include <stdexcept>

#include "stemmer/libstemmer.h"

class CustomStemmer {
public:
    virtual std::string getStem(const std::string &word) = 0;
    virtual ~CustomStemmer() { }
};

class SnowballStemmer : public CustomStemmer {
// Uses Snowball stemming library
public:    
    
    SnowballStemmer() {
       handle_ = sb_stemmer_new("english", "ISO_8859_1");
       if (!handle_) {
            throw std::runtime_error("failed to initialize stemmer");
       }
    }

    ~SnowballStemmer() {
        if (handle_) {
            sb_stemmer_delete(handle_);
        }
    }

    virtual std::string getStem(const std::string &word) {
        const sb_symbol *ret = sb_stemmer_stem(handle_, (sb_symbol *)word.c_str(), word.length());
        if (!ret) {
            throw std::runtime_error("stemmer is out of memory");
        }
        return std::string((char *)ret);
    }

private:
    struct sb_stemmer *handle_;
};

#endif /* _SEARCH_STEMMER_H_ */

