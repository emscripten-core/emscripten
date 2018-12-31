#include <netdb.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifdef __EMSCRIPTEN__
#include <arpa/inet.h>
#endif

#define uint16 uint16_t

class NetworkAddress {
private:
    sockaddr_storage address;               ///< The resolved address
public:

    /**
     * Create a network address based on a unresolved host and port
     * @param hostname the unresolved hostname
     * @param port the port
     * @param family the address family
     */
    NetworkAddress(const char *hostname = "", uint16 port = 0, int family = AF_UNSPEC)
    {
        memset(&this->address, 0, sizeof(this->address));
        this->address.ss_family = family;
        this->SetPort(port);
    }

    uint16 GetPort() const;
    void SetPort(uint16 port);
};

/**
 * Get the port.
 * @return the port.
 */
uint16 NetworkAddress::GetPort() const
{
    printf("Get PORT family: %d\n", this->address.ss_family);
    switch (this->address.ss_family) {
        case AF_UNSPEC:
        case AF_INET:
            return ntohs(((const struct sockaddr_in *)&this->address)->sin_port);

        case AF_INET6:
            return ntohs(((const struct sockaddr_in6 *)&this->address)->sin6_port);

        default:
            throw 0;
    }
}

/**
 * Set the port.
 * @param port set the port number.
 */
void NetworkAddress::SetPort(uint16 port)
{
    printf("Set PORT family: %d, port: %d\n", this->address.ss_family, port);
    switch (this->address.ss_family) {
        case AF_UNSPEC:
        case AF_INET:
            ((struct sockaddr_in*)&this->address)->sin_port = htons(port);
            break;

        case AF_INET6:
            ((struct sockaddr_in6*)&this->address)->sin6_port = htons(port);
            break;

        default:
            throw 0;
    }
}

int main() {
    NetworkAddress na("127.0.0.1", 3979);
    printf("PORT: %d\n", na.GetPort());
    return 0;
}
