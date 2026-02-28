# Note journalière
Nathan Tschantz, printemps 2026.
## Février
### Vendredi 20.02
Lancement du travail de bachelor. Première recherche sur la faisabilité de l'utilisation du MM8108-EKH19. Une fonction contient bien les options recherchée: set le MCS et code correcteur LDPC.

Le MCS est settable via un fichier (chercher) sur dans module.L'activation de LDPC n'est pas exposé par le driver et aucun des utilitaires (iw, morse_cli, hostapd). Il faudrait donc modifier les appels de fonction qui set ces param directement dans le driver.

dans les source du driver:
- command.c
```c
int morse_cmd_set_fixed_transmission_rate(struct morse *mors, s32 bandwidth_mhz, s32 mcs_index,
					s8 use_sgi, s8 nss_idx)
{
	struct morse_cmd_req_set_transmission_rate req;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_SET_TRANSMISSION_RATE, 0, sizeof(req));

	req.enabled = 1;
	req.bandwidth_mhz = cpu_to_le32(bandwidth_mhz);
	req.mcs_index = cpu_to_le32(mcs_index);
	req.nss_idx = nss_idx;
	req.use_sgi = use_sgi;
	req.tx_80211ah_format = cpu_to_le32(-1);
	req.use_traveling_pilots = -1;
	req.use_stbc = -1;	
	req.use_ldpc = 1;
	//req.use_ldpc = -1;
	//MORSE_INFO(mors, "MORSE_HACK: LDPC force a 1 et MCS a 2\n");
    pr_info("MORSE_HACK: LDPC force a 1 et MCS a 2\n");
	return morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)&req, 0, 0, __func__);
}
```

- en haut de rc.c
```c
/* Enable/Disable the fixed rate (Disabled by default) */
static bool enable_fixed_rate __read_mostly = true;
module_param(enable_fixed_rate, bool, 0644);
MODULE_PARM_DESC(enable_fixed_rate, "Enable the fixed rate");

/* Set the fixed mcs (Takes effect when enable_fixed_rate is activated) */
//static int fixed_mcs __read_mostly = 4;
static int fixed_mcs __read_mostly = 2;
module_param(fixed_mcs, int, 0644);
MODULE_PARM_DESC(fixed_mcs, "Fixed MCS (only used when enable_fixed_rate is on)");
```

Setup de l'adresse IP de l'Ethernet comme expliquer dans MM6108_MM8108-Eval-Kit-User-Guide-2.8.pdf. Puisque l'adresse de l'ethernet de l'EKH19 est 192.168.8.1 (pour le Halow c'est 192.168.12.1). Donc on set notre adresse dans le même subnet, en l'occurence 192.168.8.10 masque 255.255.255.0 et donc la passerelle par défaut 192.168.8.1

7h30.
### Samedi - Mardi 21.02 - 24.02

Pour modifier le driver, cloner le repos github du fork de openWrt par Morse Micro. Il n'est pas possible de compiler uniquement le driver sans avoir le kernel openwrt. Donc suivre ce README.md pour setup et compiler le kernel https://github.com/MorseMicro/openwrt/. 

Une fois le kernel compiler les modif fait dans les fichier précédent doivent être faite dans. 

openwrt/build_dir/target-aarch64_cortex-a53_musl/linux-mediatek_filogic/morse_driver-1.16.4/...
- command.c idem précédent
- rc.c idem précédent
- usb.c ci-dessous
```c
static int morse_usb_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	int ret;
	struct morse *mors;
	struct morse_usb *musb;
	struct morse_chip_series *mors_chip_series = (struct morse_chip_series *)id->driver_info;
	const bool reset_hw = false;
	const bool reattach_hw = false;
	/* let the user know what node this device is now attached to */
	dev_info(&interface->dev,"#### MODIFIED DRIVER ####\n"); <---- ICI
	dev_info(&interface->dev,
		 "USB Morse device now attached to Morse driver (minor=%d)", interface->minor);¨
       
        ...
    	
        usb_autopm_get_interface(interface);
    #ifdef CONFIG_MORSE_ENABLE_TEST_MODES
    usb_test_fin:
    #endif
	    /***************************************************************************/
	    morse_cmd_set_fixed_transmission_rate(mors, 1, 2, 0, 0); <---- ICI
	    /***************************************************************************/
	    return 0;
    #ifdef CONFIG_MORSE_USER_ACCESS
    err_uaccess:
    
    ...

```
**ATTENTION: ces modifications sont temporaire !!! si on fait un make clean du kernel elles seront erase. a terme il faudrait donc ajouter un patch.**

Une fois ces modification faite, on peut recompiler UNIQUEMENT le driver.()les commande suivante sont éfféctuées dans morse-wifi/openwrt/
```bash
$> make package/morse_driver/compile V=s
```
Ensuite on récupère le .ko et on le copie dans le dossier temporaire sur le module wifi.
```bash
$> scp build_dir/target-aarch64_cortex-a53_musl/linux-mediatek_filogic/morse_driver-1.16.4/ipkg-aarch64_cortex-a53/kmod-morse/lib/modules/5.15.167/morse.ko root@192.168.8.1:/tmp/
```
**ATTENTION: ces modifications sont temporaire !!! au restart le .ko sera erase**


Sur l'EKH.
```bash
root@DUT-8108-EKH19-2_9_3:/tmp rmmod morse
# couper le wifi
root@DUT-8108-EKH19-2_9_3:/tmp wifi down
'radio2' is disabled
root@DUT-8108-EKH19-2_9_3:/tmp insmod morse.ko
# set une ...
root@DUT-8108-EKH19-2_9_3:/tmp ifconfig morse0 10.0.0.1 netmask 255.255.255.0 up
# relancer le wifi
root@DUT-8108-EKH19-2_9_3:/tmp wifi up
# pinger une adresse IP inexistante
root@DUT-8108-EKH19-2_9_3:/tmp ping -I morse0 -c 3 10.0.0.2
PING 10.0.0.2 (10.0.0.2): 56 data bytes

--- 10.0.0.2 ping statistics ---
3 packets transmitted, 0 packets received, 100% packet loss
root@DUT-8108-EKH19-2_9_3:/tmp dmesg | grep HACK
[  291.577635] MORSE_HACK: LDPC force a 1 et MCS a 2
```
Parfois ca ne marchait pas ^avait sembler résoudre le probléme.

mais apparement ca ca suffit
```bash
root@DUT-8108-EKH19-2_9_3:/tmp rmmod morse
root@DUT-8108-EKH19-2_9_3:/tmp insmod morse.ko
root@DUT-8108-EKH19-2_9_3:/tmp dmesg | grep HACK
[   56.327830] MORSE_HACK: LDPC force a 1 et MCS a 2
```

sais plus combien de temps ~10h
### Vendredi 27.02

- Clarifier la structure du projet: 
	- Encodeur vidéo: donné, a connecter a la jetson
	- jetson/OS : recupérer le stream vidéo et l'envoyer au module wifi 
	- module wifi (TX): choix a faire, si utilisable on garde l'EKH19 (sinon EKH05-01)
	- module wifi (RX): On utilisera l'EKH05-01 (avec RTOS) pour recupèrer et l'envoyer 
		au PC pour l'image
	- Display 		  : récuperer l'image et la streamer

A. IP based UDP Broadcast EKH19 (ou 05 selon capacité) + RX en monitor mode en utilisant libpicap

B. Raw broadcast + LDPC COMPLIQUÉ car fonction cacher dans le binaire.

C. utiliser les examples...

a faire: 
- installer et prendre en main le SDK mm-iot-SDK et l'ouvrir dans
STM32cubeIDE
- réaliser le cahier des charges
- faire l'introduction 
- checker si on peux desactiver l'encryption sur le ekh19 (et le mettre en monitor mode)

### Samedi 28.02
- fini la première version du cahier des charges.

**Setup de l'environnement de développement (MM IoT SDK & STM32CubeIDE sous Windows)**

Voici les étapes réalisées pour configurer la chaîne de cross compilation sous Windows avec STM32CubeIDE :

**1. Récupération du SDK (Git)**

* Fork du dépôt officiel `MorseMicro/mm-iot-sdk` sur GitHub.
* Clonage local dans `C:\Users\natha\OneDrive\Bureau\TB\mm-iot-sdk`.
* Téléchargement des dépendances vitales (FreeRTOS, lwIP, mbedTLS) via la commande :
`git submodule update --init --recursive`

**2. Importation dans STM32CubeIDE**

* Création du Workspace `C:\Users\natha\OneDrive\Bureau\TB\workspace_mm`.
* Le SDK n'utilisant pas la structure standard de STM32cubeIDE, l'importation a été faite via : **File > New > Makefile Project with Existing Code**.
* Sélection du dossier cible pour l'EKH05 : `\mm-iot-sdk\examples\ping\targets\mm-mm6108-ekh05`.
* Sélection de la toolchain : **ARM Cross GCC**.

**3. Configuration du compilateur sous Windows (Variables d'environnement)**

* Windows ne possédant pas nativement `make`, il a fallu lier les outils internes de STM32CubeIDE au projet.
* Dans `Properties > C/C++ Build > Environment`, ajout de deux chemins en tête de la variable `PATH` :
* Le chemin vers `make.exe` (situé dans `C:\ST\STM32CubeIDE_1.19.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.make.win32_2.2.0.202409170845\tools\bin`).
* Le chemin vers `arm-none-eabi-gcc.exe` (situé dans `C:\ST\STM32CubeIDE_1.19.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.13.3.rel1.win32_1.0.0.202411081344\tools\bin`).

**4. Modification des Makefiles (Adaptation Linux -> Windows)**

* Le SDK force nativement l'utilisation d'une toolchain spécifique (10.3) située dans les répertoires Linux (`/opt/`).
* Modification du fichier `\mm-iot-sdk\framework\mkcore-arm-cortex-m33f.mk` pour empecher la recherche de ces dossiers et utiliser la `PATH` de Windows :
```mk
# Configure the toolchain
TOOLCHAIN_VERSION ?= 10.3-2021.07

# Try to find the toolchain if not already specified
==================== On commente ca ====================
#ifeq ($(TOOLCHAIN_DIR),)
#    directory_exists = $(shell [ -d $(1) ] && echo "exists")
#    TOOLCHAIN_DIR := /opt/morse/gcc-arm-none-eabi-$(TOOLCHAIN_VERSION)
#    ifeq ($(call directory_exists,$(TOOLCHAIN_DIR)),)
#        TOOLCHAIN_DIR := /opt/gcc-arm-none-eabi-$(TOOLCHAIN_VERSION)
#        ifeq ($(call directory_exists,$(TOOLCHAIN_DIR)),)
#            $(error Unable to find arm-none-eabi-$(TOOLCHAIN_VERSION) toolchain)
#        endif
#   endif
#else
#	TOOLCHAIN_DIR := $(TOOLCHAIN_DIR)
#endif
========================================================
# --- MODIFICATION POUR WINDOWS / STM32CubeIDE ---
# On ignore la recherche des dossiers Linux /opt/...
# et on laisse le PATH système de STM32CubeIDE trouver le compilateur.
TOOLCHAIN_DIR := 
TOOLCHAIN_BASE := arm-none-eabi-

======================= et ca ==========================
#TOOLCHAIN_BASE := $(TOOLCHAIN_DIR)/bin/arm-none-eabi-
========================================================

CC := "$(TOOLCHAIN_BASE)gcc"
CXX := "$(TOOLCHAIN_BASE)g++"
AS := $(CC) -x assembler-with-cpp
OBJCOPY := "$(TOOLCHAIN_BASE)objcopy"
```

La compilation fonctionne, le .elf est bien générer.
(les .elf ce trouve dans `C:\Users\natha\OneDrive\Bureau\TB\mm-iot-sdk\examples\ping\targets\mm-mm6108-ekh05`)

## Mars

## Avril

## Mai

### Mecredi 20.05 15h00
**Rendu intermédiaire** 

## Juin
## Juillet

### Jeudi 23.07 avant 11h00
**Rendu final du rapport**