// =============================================================================
// File Name: NetValue.hpp
// Description: Provides a universal container for values being received over
//              the network
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

class NetValue {
public:
    explicit NetValue(unsigned char type = 0);
    virtual ~NetValue();

    NetValue(const NetValue&) = delete;
    NetValue& operator=(const NetValue& rhs);

    // Returns type ID of internal value storage
    unsigned char getType() const;

    /* Delete the underlying storage for the value and reallocate it based on
     * the type ID passed in. this->type will be updated to the new type. If no
     * argument is given, The current value of this->type is used.
     */
    void reallocValue(unsigned char type = 0);

    // Copies the memory pointed to by 'value' to internal storage
    void setValue(void* value);

    // Returns pointer to internal value storage
    void* getValue() const;

private:
    void allocValue();
    void freeValue();

    unsigned char m_type;
    void* m_value;
};
