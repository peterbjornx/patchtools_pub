Program for encrypting and decrypting Pentium II microcode patches.
Written by Peter Bosch <public@pbx.sh>.

# Disclaimer
Patchfiles produced by this program might crash or damage the system they are
loaded onto and the author takes no responsibility for any damages resulting
from use of the software.

Only public resources and publically available hardware were used by the author
to produce this program.

# Key material
The program needs a 32 bit base key to work, the file cpukeys.c lists the
various unique keys used by certain CPU models, and by careful analysis of the
code you should be able to find these from encrypted patchfiles using a partial
bruteforce attack.

# MSRAM contents
The MSRAM contents are scrambled, and to edit them you need to descramble them.
An example implementation of this can be found at
https://github.com/peterbjornx/p6tools

# Usage
	patchtools [-dec] [-p <patch.dat>] [-i <config.txt>]


		-h                Print this message and exit

		-e                Extract a patch to a configuration and
		                  MSRAM hexdump file

		-c                Create a patch from a configuration and
		                  MSRAM hexdump file

		-d                Dump the patch contents and keys to the
		                  console after encrypting or decrypting.

		-p <patch.dat>    Specifies the path of the patchfile to
		                  create or decrypt. When encrypting this
		                  option is not required as the program
		                  will use the path of the configuration
		                  file to generate the output path.

		-i <config.txt>   Specifies the path of the config file
		                  to use or extract. When extracting this
		                  option is not required as the program
		                  will use the path of the patch file to
		                  generate the output path.


# More information
More information about the patch format can be found at
 https://twitter.com/peterbjornx/status/1321653489899081728
