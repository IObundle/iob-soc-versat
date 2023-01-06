Clean implementation of crypto algorithms. 

Some modifications were performed, but generally the majority of the code was taken as is from the following opensource projects:

* sha2 - Public domain, from [PQClean](https://github.com/PQClean/PQClean/tree/master)

* aes, inside mceliece - Public domain, from [PQClean](https://github.com/PQClean/PQClean/tree/master)

* mceliece - Public domain, from [Classic McEliece](https://classic.mceliece.org/impl.html) and [PQClean](https://github.com/PQClean/PQClean/tree/master) (the SHAKE algorithm, instead of using the Keccak library as intended by the Classic McEliece code)