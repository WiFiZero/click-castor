#ifndef CLICK_CRYPTO_HH
#define CLICK_CRYPTO_HH
#include <click/element.hh>
#include "../protswitch/samanagement.hh"
#include <botan/botan.h>
#include "castor.hh"

CLICK_DECLS

typedef Botan::SecureVector<Botan::byte> SValue;
typedef Botan::Private_Key PrivateKey;
typedef Botan::Public_Key PublicKey;
typedef Botan::SymmetricKey SymmetricKey;

class Crypto: public Element {

public:
	Crypto();
	~Crypto();

	const char *class_name() const { return "Crypto"; }
	const char *port_count() const { return PORTS_0_0; }
	const char *processing() const { return AGNOSTIC; }
	int configure(Vector<String>&, ErrorHandler*);

	/**
	 * Returns a new SymmetricKey instance or NULL if no corresponding key exists
	 */
	const SymmetricKey* getSharedKey(IPAddress) const;
	SValue encrypt(const SValue&, const SymmetricKey&) const;
	SValue decrypt(const SValue&, const SymmetricKey&) const;

	// FIXME Crypto shouldn't have to know about Hash typedef
	void hash(Hash hash, const uint8_t* data, uint8_t length) const;
	SValue random(int bytes) const;
	SValue hash(const SValue& data) const;

	void testSymmetricCrypt(SValue, IPAddress) const;

private:
	SAManagement* sam;
	std::string algo;
	Botan::InitializationVector iv;
	Botan::HashFunction* hashFunction;
};

CLICK_ENDDECLS
#endif
