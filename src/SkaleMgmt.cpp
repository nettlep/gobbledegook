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

#include <thread>
#include <mutex>

#include "SkaleMgmt.h"
#include "Utils.h"
#include "Logger.h"

#include <sstream>

namespace ggk {

	// Declared in =.h For sake of Thread-safety <<<---
	// std::mutex SkaleMutex;

	// Our event thread listens for events coming from the adapter and deals with them appropriately
	// std::thread SkaleAdapter::eventThread;

	/*

	// Our thread interface, which simply launches our the thread processor on our SkaleAdapter instance
	void runEventThread()
	{
		SkaleAdapter::getInstance().runEventThread();
	}
	*/

	// Event processor, responsible for receiving events from the Skale socket
	//
	// This mehtod should not be called directly. Rather, it runs continuously on a thread until the server shuts down
	//
	// It isn't necessary to disconnect manually; the Skale socket will get disocnnected automatically at before this method returns
	static int16_t SkaleAdapter::LeePesoHW()
	{
		//PesoRaw =  rand() % 100 - 50; 
		int16_t TmpLeePesoRaw =  0x0000;
		return  TmpLeePesoRaw;
	}

	static std::string SkaleAdapter::SkaleResponce()
	{
		// Ojo: La creacion del objeto lock "lk", Bloquea SkaleMutex
		std::lock_guard<std::mutex> lk(SkaleMutex); 
		Logger::trace("SkaleResponce locks SkaleMutex to read")
		// Actualiza informacion
		// RespAskedAllredySent = true;
		return WeightReport; 
	}

<<<<<<< HEAD
	static bool SkaleAdapter::SkaleProcKmd(std::string SkaleKmnd)
	{
		// Verify Parity
		char xor = SkaleKmnd[0] ;          
		for(int i=1; i<=5; i++)            // Calcula xor
			{ xor = xor ^ SkaleKmnd[i]; }
		if ( SkaleKmnd[6] <> xor )         // command corrupted
			{ return false; }              // command corrupted --> Abort
		// Otherwise... process
		// Ojo: La creacion del objeto lock "lk", Bloquea SkaleMutex
		std::lock_guard<std::mutex> lk(SkaleMutex); 
		Logger::trace("SkaleResponce locks SkaleMutex to read")
		// Actualiza informacion
		switch(SkaleKmnd[1]) {
			case 0x0A :  //LED
				statement(s);
			break;  
			case 0x0B :  //Timer 
				// Null Action for now
			break;
			case 0x0F :  //Tare
				statement(s);
			break;  
			default : // command corrupted --> Abort 
				return false;
		}
		return true;
	}

	void SkaleAdapter::Tare()
	{
		Logger::trace("Entering the SkaleAdapter Tare");

			// Guardar informacion
			// Ojo: La creacion del objeto lock "lk", Bloquea SkaleMutex
			std::lock_guard<std::mutex> lk(SkaleMutex); 
			Logger::trace("Skale Tare locks SkaleMutex to Udate")
			// Actualiza informacion de Tara

		// 	PesoRaw	       = ????                 Peso actual se asume actualizado
			PesoConTara	   = 0x00;             // Se reportara Cero
			OffsetPaTara   = PesoRaw;          // PesoCrudo actual es la nueva base (offset)

			WeightStable   = true;             // Se asume estabilidad x un momento
			PesoRawAntes   = PesoRaw;
			DiferenciaPeso = 0x00;

			WeightReport[1] = ESkaleStable;    // 2o byte = Parm Estabilidad
			
			std::ostringstream TmpStringLen2;

		//	WeightReport[2] = PesoConTara;     // 3o y 4o. bytes Diferencia Peso 
			TmpStringLen2 = "";
			TmpStringLen2 << std::setfill('0') << std::setw(4) << std::hex << PesoConTara;
			WeightReport.replace(2, 2, TmpStringLen2); // str.replace(pos,len,str2); 

		//	WeightReport[4] = DiferenciaPeso;  // 5o y 6o. bytes Diferencia Peso 
			TmpStringLen2 = "";
			TmpStringLen2 << std::setfill('0') << std::setw(4) << std::hex << DiferenciaPeso;
			WeightReport.replace(4, 2, TmpStringLen2); // str.replace(pos,len,str2);  		           

			// Calcular y actualizar nueva paridad
			char xor = WeightReport[0];        
			for(int i=1; i<=5; i++)  // Calcula xor
				{ xor = xor ^ WeightReport[i]; }
			WeightReport[6] = xor;
			// Ojo: El termino del scope y destruccion del objeto, libera SkaleMutex
			Logger::trace("Leaving the SkaleAdapter Tare & Unlock SkaleMutex");
	}

	void SkaleAdapter::runUpdWeightThread()
	{
		Logger::trace("Entering the SkaleAdapter runUpdateThread");
		// first cicle will run with default values

		int16_t            TmpPesoRaw;   
		std::ostringstream TmpStringLen2 = '';

		// 03=Decent Type CE=weightstable 0000=weight 0000=Change =XorValidation
		//	static std::string  WeightReport = "\x03\xCE\x00\x00\x00\x00\xCD"; 
		//                                      0-1o 1-2o 2-Peso 4-Dif   6-xor    

		while ( true )	// Continuo pruebita OJO <---- No todas las actualizaciones se envian al Cliente
		{
			// Pace the cicles to avoid waist CPU
			std::this_thread::sleep_for(kRescanTimeMS);
			
			// Info Anterior contenida en Tmp-Variables

			// Procesa la informacion nueva Info Solo si ya se envio una
			// respuesta solicitada por el clente "RespAskedAllredySent"
			// if (TmpRespAskedAllredySent)
				// continue;

			// Para no consumir tiempo se lee en Var temporal
			TmpPesoRaw = SkaleAdapter::LeePesoHW(); 

			if ( true ) {   // Solo para limitar scope

				Logger::trace("runUpdateThread locks SkaleMutex to Update")
				//Ahora si se actualiza Informacion
				std::lock_guard<std::mutex> lk(SkaleMutex); 

				PesoRaw = TmpPesoRaw;
				//  Ojo: No asumir que si ahora es igual siempre a sido igual
				if ( PesoRaw == PesoRawAntes )
				{
					WeightStable = true;                          // pudo haber sido inestable
					WeightReport[1] = ESkaleStable;               // 2o byte = Parm Estabilidad

				//	PesoConTara     = PesoRaw - OffsetPaTara;        No cambio 
					DiferenciaPeso  = 0x00;                       // si pudo haber cambiado
				//	PesoRawAntes    = PesoRaw;                       No cambio 

				//	WeightReport[2] = PesoConTara;                   Peso reportado no cambio 3o y 4o. bytes 
				//	TmpStringLen2 = "";
				//	TmpStringLen2 << std::setfill('0') << std::setw(4) << std::hex << PesoConTara;
				//	WeightReport.replace(2, 2, TmpStringLen2);    // str.replace(pos,len,str2); 

				//	WeightReport[4] = DiferenciaPeso;             // 5o y 6o. bytes Diferencia Peso 
					TmpStringLen2 = "";
					TmpStringLen2 << std::setfill('0') << std::setw(4) << std::hex << DiferenciaPeso;
					WeightReport.replace(4, 2, TmpStringLen2);    // str.replace(pos,len,str2);  	
				}
				else
				{
					WeightStable = false; 
					WeightReport[1] = ESkaleChning;               // 2o byte = Parm Estabilidad
					// Nuevo peso a reportar
					PesoConTara     = PesoRaw - OffsetPaTara;
					DiferenciaPeso  = PesoRaw - PesoRawAntes;
					PesoRawAntes    = PesoRaw;                    // Ojo: A comparar en siguiente ciclo

				//	WeightReport[2] = PesoConTara;                   Nuevo peso 3o y 4o. bytes 
					TmpStringLen2 = "";
					TmpStringLen2 << std::setfill('0') << std::setw(4) << std::hex << PesoConTara;
					WeightReport.replace(2, 2, TmpStringLen2);    // str.replace(pos,len,str2); 

				//	WeightReport[4] = DiferenciaPeso;             // Nueva Diferencia a reportar 5o y 6o. bytes 
					TmpStringLen2 = "";
					TmpStringLen2 << std::setfill('0') << std::setw(4) << std::hex << DiferenciaPeso;
					WeightReport.replace(4, 2, TmpStringLen2);    // str.replace(pos,len,str2);  	
				}
			//  Calcular y actualizar nueva paridad
				char xor = WeightReport[0];        
				for(int i=1; i<=5; i++)  // Calcula xor
					{ xor = xor ^ WeightReport[i]; }
				WeightReport[6] = xor;

			}   // Termino del scope SkaleMutex
			// Ojo: Al termino del scope y destruccion del objeto, libera SkaleMutex
			Logger::trace("runUpdateThread unlocks SkaleMutex");
		}
		Logger::trace("Leaving the SkaleAdapter runUpdateThread thread");
	}
	
=======
void SkaleAdapter::Tare()
{
	Logger::trace("Entering the SkaleAdapter Tare");
/*
	// first cicle will run with default values

	// first time no blocking to update info
	static bool    TmpRespAskedAllredySent = true;
	static int16_t TmpPesoRaw        = 0x0000;   
	static int16_t TmpPesoRawAntes      = 0x0000;
	static int16_t TmpPesoConTara    = 0x0000;
	static int16_t TmpOffsetPaTara   = 0x0000;
	static int16_t TmpDiferenciaPeso = 0x0000;
	static bool    TmpWeightStable   = true;
	static bool    TmpLedOn          = false;
	static bool    TmpGramsOn        = true;
	static bool    TmpTimerOn        = false;
	// 03=Decent Type CE=weightstable 0000=weight 0000=Change =XorValidation
	static std::string  TmpWeightReport = "\x03\xCE\x00\x00\x00\x00\xCD"; 
	//                                     0-1o 1-2o 2-Peso 4-Dif   6-xor    
*/
		// Guardar informacion
		// Ojo: La creacion del objeto lock "lk", Bloquea SkaleMutex
		std::lock_guard<std::mutex> lk(SkaleMutex); 
		Logger::trace("Skale Tare locks SkaleMutex to Udate")
		// Actualiza informacion de Tara

	// 	PesoRaw	       = ????                 Peso actual se asume actualizado
		PesoConTara	   = 0x00;             // Se reportara Cero
		OffsetPaTara   = PesoRaw;          // PesoCrudo actual es la nueva base (offset)

		WeightStable   = true;             // Se asume estabilidad x un momento
		PesoRawAntes   = PesoRaw;
		DiferenciaPeso = 0x00;

		WeightReport[1] = ESkaleStable;    // 2o byte = Parm Estabilidad
		WeightReport[2] = PesoConTara;            
		WeightReport[4] = DiferenciaPeso;  // 5o y 6o. bytes Diferencia Peso 
		// Calcular y actualizar nueva paridad
		char xor = WeightReport[0];        
		for(int i=1; i<=5; i++)  // Calcula xor
			{ xor = xor ^ WeightReport[i]; }
		WeightReport[6] = xor;
		// Ojo: El termino del scope y destruccion del objeto, libera SkaleMutex

		Logger::trace("Leaving the SkaleAdapter Tare & Unlock SkaleMutex");
}

void SkaleAdapter::runUpdWeightThread()
{
	Logger::trace("Entering the SkaleAdapter runUpdateThread");
	// first cicle will run with default values

	// first time no blocking to update info
	static int16_t TmpPesoRaw        = 0x0000;   

	// 03=Decent Type CE=weightstable 0000=weight 0000=Change =XorValidation
//	static std::string  WeightReport = "\x03\xCE\x00\x00\x00\x00\xCD"; 
	//                                     0-1o 1-2o 2-Peso 4-Dif   6-xor    

	while ( true )	// Continuo pruebita OJO <---- No todas las actualizaciones se envian al Cliente
	{
		// Pace the cicles to avoid waist CPU
		std::this_thread::sleep_for(kRescanTimeMS);
		
		// Info Anterior contenida en Tmp-Variables

		// Procesa la informacion nueva Info Solo si ya se envio una
		// respuesta solicitada por el clente "RespAskedAllredySent"
		// if (TmpRespAskedAllredySent)
			// continue;

		// Para no consumir tiempo se lee en Var temporal
		TmpPesoRaw = SkaleAdapter::LeePesoHW(); 

		if ( true ) {   // Solo para limitar scope

			//Ahora si se actualiza Informacion
			std::lock_guard<std::mutex> lk(SkaleMutex); 
			Logger::trace("runUpdateThread locks SkaleMutex to Update")

			PesoRaw = TmpPesoRaw;
			//  Ojo: No asumir que si ahora es igual siempre a sido igual
			if ( PesoRaw == PesoRawAntes )
			{
				WeightStable = true;                          // pudo haber sido inestable
				WeightReport[1] = ESkaleStable;               // 2o byte = Parm Estabilidad

			//	PesoConTara     = PesoRaw - OffsetPaTara;        No cambio 
			//	WeightReport[2] = PesoConTara;                   Peso reportado no cambio

				DiferenciaPeso  = 0x00;                       // si pudo haber cambiado
				WeightReport[4] = DiferenciaPeso;             // 5o y 6o. bytes Diferencia Peso 
				
			//	PesoRawAntes = PesoRaw;                         No cambio 
			}
			else
			{
				WeightStable = false; 
				WeightReport[1] = ESkaleChning;                 // 2o byte = Parm Estabilidad
				// Nuevo peso a reportar
				PesoConTara     = PesoRaw - OffsetPaTara;
				WeightReport[2] = PesoConTara;               // 3er y 4o bytes Parm Peso Actual
				// Nueva Diferencia a reportar
				DiferenciaPeso  = PesoRaw - PesoRawAntes;
				WeightReport[4] = DiferenciaPeso;            // 5o y 6o. bytes Parm Diferencia Peso 
				PesoRawAntes    = PesoRaw;                   // Ojo: A comparar en siguiente ciclo
			}
			// Calcular y actualizar nueva paridad
			char xor = WeightReport[0];        
			for(int i=1; i<=5; i++)  // Calcula xor
				{ xor = xor ^ WeightReport[i]; }
			WeightReport[6] = xor;

			// Ojo: El termino del scope y destruccion del objeto, libera SkaleMutex
			Logger::trace("runUpdateThread unlocks SkaleMutex");
		}
	}

	Logger::trace("Leaving the SkaleAdapter runUpdateThread thread");
}

>>>>>>> fa57b949569fa7d9f3649af1d36664409ff31183
}; // namespace ggk



		/* PPPOOL de Codigo

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
	//SkaleSocket.disconnect();

	// Logger::trace("Leaving the SkaleAdapter event thread");
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
	auto proximo = ahora + 2000ms;

	std::lock_guard<std::mutex> lk(SkaleInfoMutex);
	conditionalValue = commandCode;
	cvSkaleInfo.notify_one();
	std::this_thread::sleep_until(proximo);
}
*/