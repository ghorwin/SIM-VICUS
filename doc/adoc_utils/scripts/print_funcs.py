from colorama import *

USE_COLORS = True

def printError(msg):
	if USE_COLORS:
		print Fore.RED + Style.BRIGHT + msg + Fore.RESET + Style.RESET_ALL
	else:
		print msg

def printWarning(msg):
	if USE_COLORS:
		print Fore.YELLOW + Style.BRIGHT + msg + Fore.RESET + Style.RESET_ALL
	else:
		print msg

def printNotification(msg):
	if USE_COLORS:
		print Fore.GREEN + Style.BRIGHT + msg + Fore.RESET + Style.RESET_ALL
	else:
		print msg
