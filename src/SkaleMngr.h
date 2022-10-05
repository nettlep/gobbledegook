// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file in the root of the source tree.

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// >>
// >>>  INSIDE THIS FILE
// >>
//
// This is the top-level interface for the Scale full description and implementation.
//
// >>
// >>>  DISCUSSION
// >>
//
// See the discussion at the top of SkaleCntrl.cpp
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma once

#include <stdint.h>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <random>

#include "Logger.h"

namespace ggk {

// Globa var used or sake of Thread-safety <<<---
std::mutex SkaleMutex;

class SkaleAdapter
{
public:

	//
	// Constants
	//

	// Wait time before new scale values update cycle
	const std::chrono::milliseconds kRescanTimeMS = std::chrono::milliseconds(33)
/*
	// How long to wait for a response event for commands sent to the adapter
	static const int kMaxEventWaitTimeMS = 1000;

	// A constant referring to a 'non-controller' (for commands that do not require a controller index)
	static const uint16_t kNonController = 0xffff;

	// Command code names
	static const int kMinCommandCode = 0x0001;
	static const char * const kCommandCodeNames[kMaxCommandCode + 1];

	// Event type names
	static const int kMinEventType = 0x0001;
	static const char * const kEventTypeNames[kMaxEventType + 1];
*/

	//
	// Types
	//

	enum SkaleStability : uint8_t
	{
		ESkaleStable = 0xCE,   // weight stable
		ESkaleChning = 0xCA,   // weight changing
	};

	enum SkaleKomds : uint8_t
	{
		ESkaleLEDnGrKmd = 0x0A,   // LED and grams on/off
		ESkaleTimerKmd  = 0x0B,   // Timer on/off
		ESkaleTareKmd   = 0x0F    // Tare
	};

	static std::string SkaleAdapter::SkaleResponce();

	static bool SkaleAdapter::SkaleProcKmd(std::string SkaleKmnd);

	void SkaleAdapter::runUpdWeightThread();
	struct SkaleHW
	{


		std::string debugText()
		{
			std::string text = "";
			text += "> Mensaje???\n";
			text += "  + Command code       : " + Utils::hex(code) + " (" + SkaleAdapter::kCommandCodeNames[code] + ")\n";
			text += "  + Controller Id      : " + Utils::hex(controllerId) + "\n";
			text += "  + Data size          : " + std::to_string(dataSize) + " bytes";
			return text;
		}
	} __attribute__((packed));

	struct CommandCompleteEvent
	{
		SkaleHeader header;
		uint16_t commandCode;
		uint8_t status;

		CommandCompleteEvent(const std::vector<uint8_t> &data)
		{
			*this = *reinterpret_cast<const CommandCompleteEvent *>(data.data());
			toHost();

			// Log it
			Logger::debug(debugText());
		}

		void toNetwork()
		{
			header.toNetwork();
			commandCode = Utils::endianToSkale(commandCode);
		}

		void toHost()
		{
			header.toHost();
			commandCode = Utils::endianToHost(commandCode);
		}

		std::string debugText()
		{
			std::string text = "";
			text += "> Command complete event\n";
			text += "  + Event code         : " + Utils::hex(header.code) + " (" + SkaleAdapter::kEventTypeNames[header.code] + ")\n";
			text += "  + Controller Id      : " + Utils::hex(header.controllerId) + "\n";
			text += "  + Data size          : " + std::to_string(header.dataSize) + " bytes\n";
			text += "  + Command code       : " + Utils::hex(commandCode) + " (" + SkaleAdapter::kCommandCodeNames[commandCode] + ")\n";
			text += "  + Status             : " + Utils::hex(status);
			return text;
		}
	} __attribute__((packed));

	struct CommandStatusEvent
	{
		SkaleHeader header;
		uint16_t commandCode;
		uint8_t status;

		CommandStatusEvent(const std::vector<uint8_t> &data)
		{
			*this = *reinterpret_cast<const CommandStatusEvent *>(data.data());
			toHost();

			// Log it
			Logger::debug(debugText());
		}

		void toNetwork()
		{
			header.toNetwork();
			commandCode = Utils::endianToSkale(commandCode);
		}

		void toHost()
		{
			header.toHost();
			commandCode = Utils::endianToHost(commandCode);
		}

		std::string debugText()
		{
			std::string text = "";
			text += "> Command status event\n";
			text += "  + Event code         : " + Utils::hex(header.code) + " (" + SkaleAdapter::kEventTypeNames[header.code] + ")\n";
			text += "  + Controller Id      : " + Utils::hex(header.controllerId) + "\n";
			text += "  + Data size          : " + std::to_string(header.dataSize) + " bytes\n";
			text += "  + Command code       : " + Utils::hex(commandCode) + " (" + SkaleAdapter::kCommandCodeNames[commandCode] + ")\n";
			text += "  + Status             : " + Utils::hex(status) + " (" + SkaleAdapter::kStatusCodes[status] + ")";
			return text;
		}
	} __attribute__((packed));

	struct DeviceConnectedEvent
	{
		SkaleHeader header;
		uint8_t address[6];
		uint8_t addressType;
		uint32_t flags;
		uint16_t eirDataLength;

		DeviceConnectedEvent(const std::vector<uint8_t> &data)
		{
			*this = *reinterpret_cast<const DeviceConnectedEvent *>(data.data());
			toHost();

			// Log it
			Logger::debug(debugText());
		}

		void toNetwork()
		{
			header.toNetwork();
			flags = Utils::endianToSkale(flags);
			eirDataLength = Utils::endianToSkale(eirDataLength);
		}

		void toHost()
		{
			header.toHost();
			flags = Utils::endianToHost(flags);
			eirDataLength = Utils::endianToHost(eirDataLength);
		}

		std::string debugText()
		{
			std::string text = "";
			text += "> DeviceConnected event\n";
			text += "  + Event code         : " + Utils::hex(header.code) + " (" + SkaleAdapter::kEventTypeNames[header.code] + ")\n";
			text += "  + Controller Id      : " + Utils::hex(header.controllerId) + "\n";
			text += "  + Data size          : " + std::to_string(header.dataSize) + " bytes\n";
			text += "  + Address            : " + Utils::bluetoothAddressString(address) + "\n";
			text += "  + Address type       : " + Utils::hex(addressType) + "\n";
			text += "  + Flags              : " + Utils::hex(flags) + "\n";
			text += "  + EIR Data Length    : " + Utils::hex(eirDataLength);
			if (eirDataLength > 0)
			{
				text += "\n";
				text += "  + EIR Data           : " + Utils::hex(reinterpret_cast<uint8_t *>(&eirDataLength) + 2, eirDataLength);
			}
			return text;
		}
	} __attribute__((packed));

	struct DeviceDisconnectedEvent
	{
		SkaleHeader header;
		uint8_t address[6];
		uint8_t addressType;
		uint8_t reason;

		DeviceDisconnectedEvent(const std::vector<uint8_t> &data)
		{
			*this = *reinterpret_cast<const DeviceDisconnectedEvent *>(data.data());
			toHost();

			// Log it
			Logger::debug(debugText());
		}

		void toNetwork()
		{
			header.toNetwork();
		}

		void toHost()
		{
			header.toHost();
		}

		std::string debugText()
		{
			std::string text = "";
			text += "> DeviceDisconnected event\n";
			text += "  + Event code         : " + Utils::hex(header.code) + " (" + SkaleAdapter::kEventTypeNames[header.code] + ")\n";
			text += "  + Controller Id      : " + Utils::hex(header.controllerId) + "\n";
			text += "  + Data size          : " + std::to_string(header.dataSize) + " bytes\n";
			text += "  + Address            : " + Utils::bluetoothAddressString(address) + "\n";
			text += "  + Address type       : " + Utils::hex(addressType) + "\n";
			text += "  + Reason             : " + Utils::hex(reason);
			return text;
		}
	} __attribute__((packed));

	struct AdapterSettings
	{
		uint32_t masks;

		void toHost()
		{
			masks = Utils::endianToHost(masks);
		}

		bool isSet(SkaleControllerSettings mask)
		{
			return (masks & mask) != 0;
		}

		// Returns a human-readable string of flags and settings
		std::string toString()
		{
			std::string text = "";
			if (isSet(ESkalePowered)) { text += "Powered, "; }
			if (isSet(ESkaleConnectable)) { text += "Connectable, "; }
			if (isSet(ESkaleFastConnectable)) { text += "FC, "; }
			if (isSet(ESkaleDiscoverable)) { text += "Discov, "; }
			if (isSet(ESkaleBondable)) { text += "Bondable, "; }
			if (isSet(ESkaleLinkLevelSecurity)) { text += "LLS, "; }
			if (isSet(ESkaleSecureSimplePairing)) { text += "SSP, "; }
			if (isSet(ESkaleBasicRate_EnhancedDataRate)) { text += "BR/EDR, "; }
			if (isSet(ESkaleHighSpeed)) { text += "HS, "; }
			if (isSet(ESkaleLowEnergy)) { text += "LE, "; }
			if (isSet(ESkaleAdvertising)) { text += "Adv, "; }
			if (isSet(ESkaleSecureConnections)) { text += "SC, "; }
			if (isSet(ESkaleDebugKeys)) { text += "DebugKeys, "; }
			if (isSet(ESkalePrivacy)) { text += "Privacy, "; }
			if (isSet(ESkaleControllerConfiguration)) { text += "ControllerConfig, "; }
			if (isSet(ESkaleStaticAddress)) { text += "StaticAddr, "; }

			if (text.length() != 0)
			{
				text = text.substr(0, text.length() - 2);
			}

			return text;
		}

		std::string debugText()
		{
			std::string text = "";
			text += "> Adapter settings\n";
			text += "  + " + toString();
			return text;
		}
	} __attribute__((packed));

	// The comments documenting these fields are very high level. There is a lot of detailed information not present, for example
	// some values are not available at all times. This is fully documented in:
	//
	//     https://git.kernel.org/pub/scm/bluetooth/bluez.git/tree/doc/mgmt-api.txt
	struct ControllerInformation
	{
		uint8_t address[6];         // The Bluetooth address
		uint8_t bluetoothVersion;   // Bluetooth version
		uint16_t manufacturer;      // The manufacturer
		AdapterSettings supportedSettings; // Bits for various supported settings (see SkaleControllerSettings)
		AdapterSettings currentSettings;   // Bits for various currently configured settings (see SkaleControllerSettings)
		uint8_t classOfDevice[3];   // Um, yeah. That.
		char name[249];             // Null terminated name
		char shortName[11];         // Null terminated short name

		void toHost()
		{
			manufacturer = Utils::endianToHost(manufacturer);
			supportedSettings.toHost();
			currentSettings.toHost();
		}

		std::string debugText()
		{
			std::string text = "";
			text += "> Controller information\n";
			text += "  + Current settings   : " + Utils::hex(currentSettings.masks) + "\n";
			text += "  + Address            : " + Utils::bluetoothAddressString(address) + "\n";
			text += "  + BT Version         : " + std::to_string(static_cast<int>(bluetoothVersion)) + "\n";
			text += "  + Manufacturer       : " + Utils::hex(manufacturer) + "\n";
			text += "  + Supported settings : " + supportedSettings.toString() + "\n";
			text += "  + Current settings   : " + currentSettings.toString() + "\n";
			text += "  + Name               : " + std::string(name) + "\n";
			text += "  + Short name         : " + std::string(shortName);
			return text;
		}
	} __attribute__((packed));

	struct VersionInformation
	{
		uint8_t version;
		uint16_t revision;

		void toHost()
		{
			revision = Utils::endianToHost(revision);
		}

		std::string debugText()
		{
			std::string text = "";
			text += "> Version information\n";
			text += "  + Version  : " + std::to_string(static_cast<int>(version)) + "\n";
			text += "  + Revision : " + std::to_string(revision);
			return text;
		}
	} __attribute__((packed));

	struct LocalName
	{
		char name[249];
		char shortName[11];

		std::string debugText()
		{
			std::string text = "";
			text += "> Local name information\n";
			text += "  + Name       : '" + std::string(name) + "'\n";
			text += "  + Short name : '" + std::string(shortName) + "'";
			return text;
		}
	} __attribute__((packed));

	//
	// Accessors
	//

	// Returns the instance to this singleton class
	static SkaleAdapter &getInstance()
	{
		static SkaleAdapter instance;
		return instance;
	}

	AdapterSettings getAdapterSettings() { return adapterSettings; }
	ControllerInformation getControllerInformation() { return controllerInformation; }
	VersionInformation getVersionInformation() { return versionInformation; }
	LocalName getLocalName() { return localName; }
	int getActiveConnectionCount() { return activeConnections; }

	//
	// Disallow copies of our singleton (c++11)
	//

	SkaleAdapter(SkaleAdapter const&) = delete;
	void operator=(SkaleAdapter const&) = delete;

	// Reads current values from the controller
	//
	// This effectively requests data from the controller but that data may not be available instantly, but within a few
	// milliseconds. Therefore, it is not recommended attempt to retrieve the results from their accessors immediately.
	void sync(uint16_t controllerIndex);

	// Connects the Skale socket if a connection does not already exist and starts the run thread
	//
	// If a connection already exists, this method will fail
	//
	// Note that it shouldn't be necessary to connect manually; any action requiring a connection will automatically connect
	//
	// Returns true if the Skale socket is connected (either via a new connection or an existing one), otherwise false
	bool start();

	// Waits for the SkaleAdapter run thread to join
	//
	// This method will block until the thread joins
	void stop();

	// Sends a command over the Skale socket
	//
	// If the Skale socket is not connected, it will auto-connect prior to sending the command. In the case of a failed auto-connect,
	// a failure is returned.
	//
	// Returns true on success, otherwise false
	bool sendCommand(SkaleHeader &request);

	// Event processor, responsible for receiving events from the Skale socket
	//
	// This mehtod should not be called directly. Rather, it runs continuously on a thread until the server shuts down
	void runEventThread();

private:
	// ???
	// first time no blocking to update info
	static bool    RespAskedAllredySent = true;
	static int16_t PesoRaw        = 0x0000;      // Grams * 10
	static int16_t PesoAntes      = 0x0000;
	static int16_t PesoAhora      = 0x0000;
	static int16_t DiferenciaPeso = 0x0000;
	static uint8_t WeightStable   = true;
	static bool    LedOn          = false;
	static bool    GramsOn        = true;
	static bool    TimerOn        = false;
	// 03=Decent Type CE=weightstable 0000=weight 0000=Change =XorValidation
	static std::string  WeightReport = "\x03\xCE\x00\x00\x00\x00\xCD"; 


	// Private constructor for our Singleton <<--- Ojo
	static int16_t SkaleAdapter::LeePesoHW();

	SkaleAdapter() : SkaleInfoLock(SkaleInfoMutex), activeConnections(0) {}

	// Uses a std::condition_variable to wait for a response event for the given `commandCode` or `timeoutMS` milliseconds.
	//
	// Returns true if the response event was received for `commandCode` or false if the timeout expired.
	//
	// Command responses are set via `setCommandResponse()`
	bool waitForCommandResponse(uint16_t commandCode, int timeoutMS);

	// Sets the command response and notifies the waiting std::condition_variable (see `waitForCommandResponse`)
	void setCommandResponse(uint16_t commandCode);

	// Our Skale Socket, which allows us to talk directly to the kernel
	SkaleSocket SkaleSocket;

	// Our event thread listens for events coming from the adapter and deals with them appropriately
	static std::thread eventThread;

	// Our adapter information
	std::condition_variable cvSkaleInfo;
	std::mutex SkaleInfoMutex;
	std::unique_lock<std::mutex> SkaleInfoLock;
	int conditionalValue;


/*
	AdapterSettings adapterSettings;
	ControllerInformation controllerInformation;
	VersionInformation versionInformation;
	LocalName localName;


	std::condition_variable cvCommandResponse;
	std::mutex commandResponseMutex;
	std::unique_lock<std::mutex> commandResponseLock;
	int conditionalValue;

	// Our active connection count
	int activeConnections;
*/

};

}; // namespace ggk