# Certificates

The certificates and keys in this directory are used by the simulator (mrs) and the test harness (mth).

>**NOTE** Certificates and keys in this directory are for testing and development purposes only.  They are  not to be used in production environments.

## Creating Certificates
Creating your own set of certificates is straightforward and can be done using `openssl`.  Follow the steps below to setup a certificate authority (CA) of your own and then use that CA cert to create server and client certificates.

>For simplicity, the certificates and keys generated for the simulator and mth follow the procedure below without passphrases protecting sensitive data.  For production purposes you, of course, should protect all certificate and key elements with passphrases and encryption as appropriate for your environment.

### Create CA certificate
1. Generate a private key.  Store it in `ca.key`.
```shell
$ openssl genrsa -out ca.key 2048
```

2. Generate a root certificate.  Store it in `ca.crt`.
```shell
$ openssl req -x509 -new -nodes -key ca.key -sha256 -days 1825 -out ca.crt
```

### Create a certificate for the simulator (the server)
1. Generate a private key.  Store it in `srv.key`.
```shell
$ openssl genrsa -out srv.key 2048
```
2. Generate a CSR (Certificate Signing Request). Store it in `srv.csr`.  (Don't bother with the extra information request such as a challenge password or company name.)
```shell
$ openssl req -new -key srv.key -out srv.csr
```
3. Generate the certificate and sign it with the CA certificate created earlier.  Store this in `srv.crt`.
```shell
$ openssl x509 -req -in srv.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out srv.crt -days 825 -sha256
```

### Create a certificate for the client app using the Magellan library
1. Generate a private key.  Store it in `client.key`.
```shell
$ openssl genrsa -out client.key 2048
```
2. Generate a CSR (Certificate Signing Request). Store it in `client.csr`.  (Don't bother with the extra information request such as a challenge password or company name.)
```shell
$ openssl req -new -key client.key -out client.csr
```
3. Generate the certificate and sign it with the CA certificate created earlier.  Store this in `client.crt`.
```shell
$ openssl x509 -req -in client.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out client.crt -days 825 -sha256
```
