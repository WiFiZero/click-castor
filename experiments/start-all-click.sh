#!/bin/bash

EXPERIMENT_DIR=/root/click-castor/experiments
NODEINFO=`dirname $0`/nodeinfo.sh

# List all nodes
NODES=`${NODEINFO} -t ID`

REBOOT='no'
NOCHECK='no'

while getopts 'rn' flag; do
  case "${flag}" in
    r) REBOOT='yes' ;;
    n) NOCHECK='yes' ;;
    *) error "Unexpected option ${flag}"
  esac
done

if [[ ${REBOOT} == 'yes' ]]; then
	echo "--- (Re)booting nodes ..."

	for n in ${NODES}; do
		echo "  ... node ${n} ..."
		pypwrctrl off `${NODEINFO} -i ${n} -t PYPWR` &>/dev/null
	done

	sleep 1

	for n in ${NODES}; do
		pypwrctrl on  `${NODEINFO} -i ${n} -t PYPWR` &>/dev/null
	done
fi

if [[ ${NOCHECK} == 'no' ]]; then
	echo "--- Checking if nodes are up ..."
	for n in ${NODES}
	do
		nodename=`${NODEINFO} -i ${n} -t DNS`
		# ':' is no-op command
		#while ! ping -c 1 ${nodename} &>/dev/null; do sleep 0.1; done
		until ssh ${nodename} : &>/dev/null; do
			sleep 1
		done
		echo "  ... ${nodename} is up!"
	done
fi

echo "--- Starting Click ..."

HOSTS=`mktemp`
${NODEINFO} -t DNS > ${HOSTS}

pssh -l root -h ${HOSTS} ${EXPERIMENT_DIR}/start-click.sh

rm ${HOSTS}
