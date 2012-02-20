#set -x -e
BASE_DIR=`dirname $0`

# Auto-detect openocd version
OPENOCD_VERSION=$(openocd --version 2>&1 | sed -ne '1p' | \
		sed "s/\\([^ ]\\+\\) \\([^ ]\\+\\) \\([^ ]\\+\\) \\([^ ]\\+\\) .*$/\\4/")
if [ "z$OPENOCD_VERSION" == "z" ]; then
	echo "ERROR: unable to detect openocd version"
	exit 1
fi
echo "openocd $OPENOCD_VERSION detected"
#OPENOCD_VERSION_MIN=$(echo $OPENOCD_VERSION | \
#		sed "s/^[0-9]\\+\\.\([0-9]\\+\)\\.[0-9]\\+$/\\1/")
OPENOCD_VERSION_MIN=$(echo $OPENOCD_VERSION | gawk -F. '{print $2 }')

if [ $OPENOCD_VERSION_MIN -le  4 ]; then
	OCD_VERSION=""
else
	OCD_VERSION=$(echo $OPENOCD_VERSION | gawk -F. '{print "-" $1 "." $2 }')
fi

INT_FILE=${BASE_DIR}/openocd/${PROGRAMMER_TYPE}.cfg
CPU_FILE=${BASE_DIR}/openocd/${PROGRAMMER_CPU}${OCD_VERSION}.cfg
echo "$CPU_FILE"

if [ ! -f ${INT_FILE} ]; then
	printf "CLDLG: Interface ${PROGRAMMER_TYPE} not supported\n";
	exit 1;
fi

if [ ! -f ${CPU_FILE} ]; then
	printf "CLDLG: CPU ${PROGRAMMER_CPU} not supported\n";
	exit 1;
fi

