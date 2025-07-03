# OpenDDS Simple Publisher/Subscriber Example

This project demonstrates a minimal Publisher/Subscriber (Pub/Sub) application using [OpenDDS](https://opendds.org/) in C++. It uses a simple `Messager::Message` topic defined in IDL and shows how to send and receive messages using OpenDDS.

## Project Structure

- **Messager.idl**: IDL definition for the `Message` topic.
- **publisher.cpp**: Publishes `Message` samples.
- **subscriber.cpp**: Subscribes and prints received `Message` samples.
- **MessageReaderListener.h/cpp**: Implements the DataReader listener for the subscriber.
- **CMakeLists.txt**: Build configuration.
- **generate_idl**: Script to generate OpenDDS/TAO code from IDL.
- **dds_tcp_conf.ini**: Example OpenDDS TCP transport configuration.

## Build Instructions

1. **Generate IDL sources:**
   ```sh
   ./generate_idl Messager
   ```

2. **Configure and build:**
   ```sh
   mkdir -p bin
   cd bin
   cmake ..
   cmake --build .
   ```

## Running the Example

Open **three terminals** and run the following in each:

1. **Terminal 1: Start the OpenDDS InfoRepo**
   ```sh
   $DDS_ROOT/dds/InfoRepo/DCPSInfoRepo -ORBEndpoint iiop://localhost:12345 -d domain_ids
   ```

2. **Terminal 2: Run the Publisher**
   ```sh
   ./bin/publisher -DCPSConfigFile ../dds_tcp_conf.ini
   ```

3. **Terminal 3: Run the Subscriber**
   ```sh
   ./bin/subscriber -DCPSConfigFile ../dds_tcp_conf.ini
   ```

## Notes

- Make sure `$DDS_ROOT` is set to your OpenDDS installation.
- The publisher will send 10 messages, one per second.
- The subscriber will print each received message to the terminal.

---

This example is intended for learning and experimenting with OpenDDS Pub/Sub