//
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file in the root of the source tree.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// >>
// >>>  INSIDE THIS FILE
// >>
//
// This is the Scale Controler
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// >>
// >>>  INSIDE THIS FILE
// >>
//
// Protocol-level code for the Bluetooth Management API, which is used to configure the Bluetooth adapter
//
// >>
// >>>  DISCUSSION
// >>
//
// This class is intended for use by `Mgmt` (see Mgmt.cpp).
//
// The Bluetooth Management API, is used to configure the Bluetooth adapter (such as enabling LE, setting the device name, etc.)
// This class uses SkaleSocket (SkaleSocket.h) for the raw communications.
//
// The information for this implementation (as well as SkaleSocket.h) came from:
//
//     https://git.kernel.org/pub/scm/bluetooth/bluez.git/tree/doc/mgmt-api.txt
//
// KNOWN LIMITATIONS:
//
// This is far from a complete implementation. I'm not even sure how reliable of an implementation this is. However, I can say with
// _complete_confidence_ that it works on my machine after numerous minutes of testing.
//
// One notable limitation is that this code doesn't work with the Bluetooth Management API in the way it was intended. The Bluetooth
// Management API treats the sending and receiving of data differently. It receives commands on the Skale socket and acts upon them.
// It also sends events on the same socket. It is important to note that there is not necessarily a 1:1 correlation from commands
// received to events generated. For example, an event can be generated when a bluetooth client disconnects, even though no command
// was sent for which that event is associated.
//
// However, for initialization, it seems to be generally safe to treat them as "nearly 1:1". The solution below is to consume all
// events and look for the event that we're waiting on. This seems to work in my environment (Raspberry Pi) fairly well, but please
// do use this with caution.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include <string.h>
#include <chrono>
#include <future>

#include "SkaleMngr.h"
#include "Utils.h"
#include "Logger.h"

namespace ggk {

// Our event thread listens for events coming from the adapter and deals with them appropriately
std::thread SkaleAdapter::eventThread;

/*
const char * const SkaleAdapter::kCommandCodeNames[kMaxCommandCode + 1] =
{
	"Invalid Command",                                   // 0x0000
	"Read Version Information Command",                  // 0x0001
	"Read Supported Commands Command",                   // 0x0002
	"Read Controller Index List Command",                // 0x0003
	"Read Controller Information Command",               // 0x0004
	"Set Powered Command",                               // 0x0005
	"Set Discoverable Command",                          // 0x0006
	"Set Connectable Command",                           // 0x0007
	"Set Fast Connectable Command",                      // 0x0008
	"Set Bondable Command",                              // 0x0009
	"Set Link Security Command",                         // 0x000A
	"Set Secure Simple Pairing Command",                 // 0x000B
	"Set High Speed Command",                            // 0x000C
	"Set Low Energy Command",                            // 0x000D
	"Set Device Class",                                  // 0x000E
	"Set Local Name Command",                            // 0x000F
	"Add UUID Command",                                  // 0x0010
	"Remove UUID Command",                               // 0x0011
	"Load Link Keys Command",                            // 0x0012
	"Load Long Term Keys Command",                       // 0x0013
	"Disconnect Command",                                // 0x0014
	"Get Connections Command",                           // 0x0015
	"PIN Code Reply Command",                            // 0x0016
	"PIN Code Negative Reply Command",                   // 0x0017
	"Set IO Capability Command",                         // 0x0018
	"Pair Device Command",                               // 0x0019
	"Cancel Pair Device Command",                        // 0x001A
	"Unpair Device Command",                             // 0x001B
	"User Confirmation Reply Command",                   // 0x001C
	"User Confirmation Negative Reply Command",          // 0x001D
	"User Passkey Reply Command",                        // 0x001E
	"User Passkey Negative Reply Command",               // 0x001F
	"Read Local Out Of Band Data Command",               // 0x0020
	"Add Remote Out Of Band Data Command",               // 0x0021
	"Remove Remote Out Of Band Data Command",            // 0x0022
	"Start Discovery Command",                           // 0x0023
	"Stop Discovery Command",                            // 0x0024
	"Confirm Name Command",                              // 0x0025
	"Block Device Command",                              // 0x0026
	"Unblock Device Command",                            // 0x0027
	"Set Device ID Command",                             // 0x0028
	"Set Advertising Command",                           // 0x0029
	"Set BR/EDR Command",                                // 0x002A
	"Set Static Address Command",                        // 0x002B
	"Set Scan Parameters Command",                       // 0x002C
	"Set Secure Connections Command",                    // 0x002D
	"Set Debug Keys Command",                            // 0x002E
	"Set Privacy Command",                               // 0x002F
	"Load Identity Resolving Keys Command",              // 0x0030
	"Get Connection Information Command",                // 0x0031
	"Get Clock Information Command",                     // 0x0032
	"Add Device Command",                                // 0x0033
	"Remove Device Command",                             // 0x0034
	"Load Connection Parameters Command",                // 0x0035
	"Read Unconfigured Controller Index List Command",   // 0x0036
	"Read Controller Configuration Information Command", // 0x0037
	"Set External Configuration Command",                // 0x0038
	"Set Public Address Command",                        // 0x0039
	"Start Service Discovery Command",                   // 0x003a
	"Read Local Out Of Band Extended Data Command",      // 0x003b
	"Read Extended Controller Index List Command",       // 0x003c
	"Read Advertising Features Command",                 // 0x003d
	"Add Advertising Command",                           // 0x003e
	"Remove Advertising Command",                        // 0x003f
	"Get Advertising Size Information Command",          // 0x0040
	"Start Limited Discovery Command",                   // 0x0041
	"Read Extended Controller Information Command",      // 0x0042
	// NOTE: The documentation at https://git.kernel.org/pub/scm/bluetooth/bluez.git/tree/doc/mgmt-api.txt) states that the command
	// code for "Set Appearance Command" is 0x0042. It also says this about the previous command in the list ("Read Extended
	// Controller Information Command".) This is likely an error, so I'm following the order of the commands as they appear in the
	// documentation. This makes "Set Appearance Code" have a command code of 0x0043.
	"Set Appearance Command"                             // 0x0043
};

const char * const SkaleAdapter::kEventTypeNames[kMaxEventType + 1] =
{
	"Invalid Event",                                     // 0x0000
	"Command Complete Event",                            // 0x0001
	"Command Status Event",                              // 0x0002
	"Controller Error Event",                            // 0x0003
	"Index Added Event",                                 // 0x0004
	"Index Removed Event",                               // 0x0005
	"New Settings Event",                                // 0x0006
	"Class Of Device Changed Event",                     // 0x0007
	"Local Name Changed Event",                          // 0x0008
	"New Link Key Event",                                // 0x0009
	"New Long Term Key Event",                           // 0x000A
	"Device Connected Event",                            // 0x000B
	"Device Disconnected Event",                         // 0x000C
	"Connect Failed Event",                              // 0x000D
	"PIN Code Request Event",                            // 0x000E
	"User Confirmation Request Event",                   // 0x000F
	"User Passkey Request Event",                        // 0x0010
	"Authentication Failed Event",                       // 0x0011
	"Device Found Event",                                // 0x0012
	"Discovering Event",                                 // 0x0013
	"Device Blocked Event",                              // 0x0014
	"Device Unblocked Event",                            // 0x0015
	"Device Unpaired Event",                             // 0x0016
	"Passkey Notify Event",                              // 0x0017
	"New Identity Resolving Key Event",                  // 0x0018
	"New Signature Resolving Key Event",                 // 0x0019
	"Device Added Event",                                // 0x001a
	"Device Removed Event",                              // 0x001b
	"New Connection Parameter Event",                    // 0x001c
	"Unconfigured Index Added Event",                    // 0x001d
	"Unconfigured Index Removed Event",                  // 0x001e
	"New Configuration Options Event",                   // 0x001f
	"Extended Index Added Event",                        // 0x0020
	"Extended Index Removed Event",                      // 0x0021
	"Local Out Of Band Extended Data Updated Event",     // 0x0022
	"Advertising Added Event",                           // 0x0023
	"Advertising Removed Event",                         // 0x0024
	"Extended Controller Information Changed Event"      // 0x0025
};

const char * const SkaleAdapter::kStatusCodes[kMaxStatusCode + 1] =
{
	"Success",                                           // 0x00
	"Unknown Command",                                   // 0x01
	"Not Connected",                                     // 0x02
	"Failed",                                            // 0x03
	"Connect Failed",                                    // 0x04
	"Authentication Failed",                             // 0x05
	"Not Paired",                                        // 0x06
	"No Resources",                                      // 0x07
	"Timeout",                                           // 0x08
	"Already Connected",                                 // 0x09
	"Busy",                                              // 0x0A
	"Rejected",                                          // 0x0B
	"Not Supported",                                     // 0x0C
	"Invalid Parameters",                                // 0x0D
	"Disconnected",                                      // 0x0E
	"Not Powered",                                       // 0x0F
	"Cancelled",                                         // 0x10
	"Invalid Index",                                     // 0x11
	"RFKilled",                                          // 0x12
	"Already Paired",                                    // 0x13
	"Permission Denied",                                 // 0x14
};
*/
// Our thread interface, which simply launches our the thread processor on our SkaleAdapter instance
void runEventThread()
{
	SkaleAdapter::getInstance().runEventThread();
}

// Event processor, responsible for receiving events from the Skale socket
//
// This mehtod should not be called directly. Rather, it runs continuously on a thread until the server shuts down
//
// It isn't necessary to disconnect manually; the Skale socket will get disocnnected automatically at before this method returns
void SkaleAdapter::runUpdateThread()
{
	Logger::trace("Entering the SkaleAdapter runUpdateThread");

	while (ggkGetServerRunState() <= ERunning /*&& SkaleSocket.isConnected() */)
	{
		// pruebita
		auto ahora = std::chrono::steady_clock::now(); 
		using std::chrono::operator""ms;
		auto proxCiclo = ahora + kAvrgRescanTimeMS;

		std::lock_guard<std::mutex> lk(SkaleInfoMutex);
		// procesar informacion

		std::this_thread::sleep_until(proxCiclo);

		
		// Read the next event, waiting until one arrives
		std::vector<uint8_t> responsePacket = std::vector<uint8_t>();
		if (!SkaleSocket.read(responsePacket))
		{
			break;
		}

		// Do we have enough to check the event code?
		if (responsePacket.size() < 2)
		{
			Logger::error(SSTR << "Invalid command response: too short");
			continue;
		}

		// Our response, as a usable object type
		uint16_t eventCode = Utils::endianToHost(*reinterpret_cast<uint16_t *>(responsePacket.data()));

		// Ensure our event code is valid
		if (eventCode < SkaleAdapter::kMinEventType || eventCode > SkaleAdapter::kMaxEventType)
		{
			Logger::error(SSTR << "Invalid command response: event code (" << eventCode << ") out of range");
			continue;
		}

		switch(eventCode)
		{
			// Command complete event
			case Mgmt::ECommandCompleteEvent:
			{
				// Extract our event
				CommandCompleteEvent event(responsePacket);

				// Point to the data following the event
				uint8_t *data = responsePacket.data() + sizeof(CommandCompleteEvent);
				size_t dataLen = responsePacket.size() - sizeof(CommandCompleteEvent);

				switch(event.commandCode)
				{
					// We just log the version/revision info
					case Mgmt::EReadVersionInformationCommand:
					{
						// Verify the size is what we expect
						if (dataLen != sizeof(VersionInformation))
						{
							Logger::error("Invalid data length");
							return;
						}

						versionInformation = *reinterpret_cast<VersionInformation *>(data);
						versionInformation.toHost();
						Logger::debug(versionInformation.debugText());
						break;
					}
					case Mgmt::EReadControllerInformationCommand:
					{
						if (dataLen != sizeof(ControllerInformation))
						{
							Logger::error("Invalid data length");
							return;
						}

						controllerInformation = *reinterpret_cast<ControllerInformation *>(data);
						controllerInformation.toHost();
						Logger::debug(controllerInformation.debugText());
						break;
					}
					case Mgmt::ESetLocalNameCommand:
					{
						if (dataLen != sizeof(LocalName))
						{
							Logger::error("Invalid data length");
							return;
						}

						localName = *reinterpret_cast<LocalName *>(data);
						Logger::info(localName.debugText());
						break;
					}
					case Mgmt::ESetPoweredCommand:
					case Mgmt::ESetBREDRCommand:
					case Mgmt::ESetSecureConnectionsCommand:
					case Mgmt::ESetBondableCommand:
					case Mgmt::ESetConnectableCommand:
					case Mgmt::ESetLowEnergyCommand:
					case Mgmt::ESetAdvertisingCommand:
					{
						if (dataLen != sizeof(AdapterSettings))
						{
							Logger::error("Invalid data length");
							return;
						}

						adapterSettings = *reinterpret_cast<AdapterSettings *>(data);
						adapterSettings.toHost();

						Logger::debug(adapterSettings.debugText());
						break;
					}
				}

				// Notify anybody waiting that we received a response to their command code
				setCommandResponse(event.commandCode);

				break;
			}
			// Command status event
			case Mgmt::ECommandStatusEvent:
			{
				CommandStatusEvent event(responsePacket);

				// Notify anybody waiting that we received a response to their command code
				setCommandResponse(event.commandCode);
				break;
			}
			// Command status event
			case Mgmt::EDeviceConnectedEvent:
			{
				DeviceConnectedEvent event(responsePacket);
				activeConnections += 1;
				Logger::debug(SSTR << "  > Connection count incremented to " << activeConnections);
				break;
			}
			// Command status event
			case Mgmt::EDeviceDisconnectedEvent:
			{
				DeviceDisconnectedEvent event(responsePacket);
				if (activeConnections > 0)
				{
					activeConnections -= 1;
					Logger::debug(SSTR << "  > Connection count decremented to " << activeConnections);
				}
				else
				{
					Logger::debug(SSTR << "  > Connection count already at zero, ignoring non-connected disconnect event");
				}
				break;
			}
			// Unsupported
			default:
			{
				if (eventCode >= kMinEventType && eventCode <= kMaxEventType)
				{
					Logger::error("Unsupported response event type: " + Utils::hex(eventCode) + " (" + kEventTypeNames[eventCode] + ")");
				}
				else
				{
					Logger::error("Invalid event type response: " + Utils::hex(eventCode));					
				}
			}
		}
	}

	// Make sure we're disconnected before we leave
	SkaleSocket.disconnect();

	Logger::trace("Leaving the SkaleAdapter event thread");
}

// Reads current values from the controller
//
// This effectively requests data from the controller but that data may not be available instantly, but within a few
// milliseconds. Therefore, it is not recommended attempt to retrieve the results from their accessors immediately.
void SkaleAdapter::sync(uint16_t controllerIndex)
{
	Logger::debug("Synchronizing version information");

	SkaleAdapter::SkaleHeader request;
	request.code = Mgmt::EReadVersionInformationCommand;
	request.controllerId = SkaleAdapter::kNonController;
	request.dataSize = 0;

	if (!SkaleAdapter::getInstance().sendCommand(request))
	{
		Logger::error("Failed to get version information");
	}

	Logger::debug("Synchronizing controller information");

	request.code = Mgmt::EReadControllerInformationCommand;
	request.controllerId = controllerIndex;
	request.dataSize = 0;

	if (!SkaleAdapter::getInstance().sendCommand(request))
	{
		Logger::error("Failed to get current settings");
	}
}

// Connects the Skale socket if a connection does not already exist and starts the run thread
//
// If the thread is already running, this method will fail
//
// Note that it shouldn't be necessary to connect manually; any action requiring a connection will automatically connect
//
// Returns true if the Skale socket is connected (either via a new connection or an existing one), otherwise false
bool SkaleAdapter::start()
{
	// If the thread is already running, return failure
	if (eventThread.joinable())
	{
		return false;
	}

	// Already connected?
	if (!SkaleSocket.isConnected())
	{
		// Connect
		if (!SkaleSocket.connect())
		{
			return false;
		}
	}

	// Create a thread to read the data from the socket
	try
	{
		eventThread = std::thread(ggk::runEventThread);
	}
	catch(std::system_error &ex)
	{
		Logger::error(SSTR << "SkaleAdapter thread was unable to start (code " << ex.code() << "): " << ex.what());
		return false;
	}

	return true;
}

// Waits for the SkaleAdapter run thread to join
//
// This method will block until the thread joins
void SkaleAdapter::stop()
{
	Logger::trace("SkaleAdapter waiting for thread termination");

	try
	{
		if (eventThread.joinable())
		{
			eventThread.join();

			Logger::trace("Event thread has stopped");
		}
		else
		{
			Logger::trace(" > Event thread is not joinable");
		}
	}
	catch(std::system_error &ex)
	{
		if (ex.code() == std::errc::invalid_argument)
		{
			Logger::warn(SSTR << "SkaleAdapter event thread was not joinable during SkaleAdapter::wait(): " << ex.what());
		}
		else if (ex.code() == std::errc::no_such_process)
		{
			Logger::warn(SSTR << "SkaleAdapter event was not valid during SkaleAdapter::wait(): " << ex.what());
		}
		else if (ex.code() == std::errc::resource_deadlock_would_occur)
		{
			Logger::warn(SSTR << "Deadlock avoided in call to SkaleAdapter::wait() (did the event thread try to stop itself?): " << ex.what());
		}
		else
		{
			Logger::warn(SSTR << "Unknown system_error code (" << ex.code() << ") during SkaleAdapter::wait(): " << ex.what());
		}
	}
}

// Sends a command over the Skale socket
//
// If the Skale socket is not connected, it will auto-connect prior to sending the command. In the case of a failed auto-connect,
// a failure is returned.
//
// Returns true on success, otherwise false
bool SkaleAdapter::sendCommand(SkaleHeader &request)
{
	// Auto-connect
	if (!eventThread.joinable() && !start())
	{
		Logger::error("SkaleAdapter failed to start");
		return false;
	}

	uint16_t code = request.code;
	uint16_t dataSize = request.dataSize;

	conditionalValue = -1;
	std::future<bool> fut = std::async(std::launch::async,
	[&]() mutable
	{
		return waitForCommandResponse(code, kMaxEventWaitTimeMS);
	});

	// Prepare the request to be sent (endianness correction)
	request.toNetwork();
	uint8_t *pRequest = reinterpret_cast<uint8_t *>(&request);

	std::vector<uint8_t> requestPacket = std::vector<uint8_t>(pRequest, pRequest + sizeof(request) + dataSize);
	if (!SkaleSocket.write(requestPacket))
	{
		return false;
	}

	return fut.get();
}

// Uses a std::condition_variable to wait for a response event for the given `commandCode` or `timeoutMS` milliseconds.
//
// Returns true if the response event was received for `commandCode` or false if the timeout expired.
//
// Command responses are set via `setCommandResponse()`
bool SkaleAdapter::waitForCommandResponse(uint16_t commandCode, int timeoutMS)
{
	Logger::debug(SSTR << "  + Waiting on command code " << commandCode << " for up to " << timeoutMS << "ms");

	bool success = cvSkaleInfo.wait_for(SkaleInfoLock, std::chrono::milliseconds(timeoutMS),
		[&]
		{
			return conditionalValue == commandCode;
		}
	);

	if (!success)
	{
		Logger::warn(SSTR << "  + Timed out waiting on command code " << Utils::hex(commandCode) << " (" << kCommandCodeNames[commandCode] << ")");
	}
	else
	{
		Logger::debug(SSTR << "  + Recieved the command code we were waiting for: " << Utils::hex(commandCode) << " (" << kCommandCodeNames[commandCode] << ")");
	}

	return success;
}

// Sets the command response and notifies the waiting std::condition_variable (see `waitForCommandResponse`)
void SkaleAdapter::setCommandResponse(uint16_t commandCode)
{
	// pruebita
	auto ahora = std::chrono::steady_clock::now(); 
    using std::chrono::operator""ms;
	auto proximo = ahora + 2000ms;

	std::lock_guard<std::mutex> lk(SkaleInfoMutex);
	conditionalValue = commandCode;
	cvSkaleInfo.notify_one();
	std::this_thread::sleep_until(proximo);
}

}; // namespace ggk