#include "notification.h"

Notification::Notification()
{
}

Notification::~Notification()
{
}

void Notification::showNotification(const std::string& message)
{
	const std::string command = "notify-send 'Pomodoro Timer' '" + message + "'";
	system(command.c_str());
	std::cout << "NOTIFICATION: " << message << std::endl;
}

void Notification::playSound(const std::string& soundFile)
{
	// Try the specified sound file first
	std::string command = "paplay " + soundFile + " 2>/dev/null";
	std::cout << "DEBUG: Playing sound with command: " << command << std::endl;
	int result = system(command.c_str());
	
	// If that fails, try with aplay
	if (result != 0) {
		command = "aplay " + soundFile + " 2>/dev/null";
		std::cout << "DEBUG: Trying aplay with command: " << command << std::endl;
		result = system(command.c_str());
	}
	
	// If both fail, try system notification sounds as fallback
	if (result != 0) {
		std::cout << "DEBUG: Sound file not found, trying system notification sounds" << std::endl;
		
		// Try common system notification sounds
		const char* systemSounds[] = {
			"/usr/share/sounds/alsa/Front_Left.wav",
			"/usr/share/sounds/ubuntu/notifications/Blip.ogg",
			"/usr/share/sounds/freedesktop/stereo/bell.oga",
			"/usr/share/sounds/speech-dispatcher/test.wav",
			"/usr/share/sounds/alsa/Noise.wav"
		};
		
		bool soundPlayed = false;
		for (size_t i = 0; i < sizeof(systemSounds) / sizeof(systemSounds[0]); ++i) {
			command = "paplay " + std::string(systemSounds[i]) + " 2>/dev/null";
			result = system(command.c_str());
			if (result == 0) {
				std::cout << "DEBUG: Successfully played system sound: " << systemSounds[i] << std::endl;
				soundPlayed = true;
				break;
			}
			
			command = "aplay " + std::string(systemSounds[i]) + " 2>/dev/null";
			result = system(command.c_str());
			if (result == 0) {
				std::cout << "DEBUG: Successfully played system sound: " << systemSounds[i] << std::endl;
				soundPlayed = true;
				break;
			}
		}
		
		// If all system sounds fail, use terminal bell as last resort
		if (!soundPlayed) {
			std::cout << "DEBUG: All sound methods failed, using terminal bell" << std::endl;
			// Use escape sequence for terminal bell
			std::cout << "\a" << std::flush;
			// Also try the speaker-test utility as backup
			system("speaker-test -t sine -f 800 -l 1 2>/dev/null &");
		}
	}
}