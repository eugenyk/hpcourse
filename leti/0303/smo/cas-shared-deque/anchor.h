#ifndef anchor_h
#define anchor_h

struct Anchor {
    unsigned int idxLeft, idxRight;
    
    Anchor() : idxLeft(0), idxRight(0)
    {
        static_check();
    }
    //    Anchor(Anchor const& a) _NOEXCEPT : idxLeft(a.idxLeft),idxRight(a.idxRight)
    //    {
    //        static_check();
    //    }
    
    Anchor(unsigned int left, unsigned int right) :idxLeft(left), idxRight(right)
    {
        static_check()  ;
    }
    
    /// Anchor equal operator
    bool operator ==( Anchor const& a) const
    {
        return idxLeft == a.idxLeft && idxRight == a.idxRight ;
    }
    
    /// Anchor non-equal operator
    bool operator !=( Anchor const& a) const
    {
        return !( *this == a ) ;
    }
    
private:
    //@cond
    static void static_check()
    {
        static_assert( sizeof(unsigned int) * 2 <= 8, "The index type must be no more than 32bit long" ) ;
        static_assert( sizeof(Anchor) <= 8, "The anchor type must be no more than 64bit long" ) ;
    }
};

#endif /* anchor_h */
