enum direction { WEST=0, EAST=1};
Semaphore mutex; // initially 1, protect critical sections
Semaphore westBlocked, eastBlocked; // one for each direction. Each initially 0, will block on first P.
int blockedCount[2]; // number of cars waiting on each side
int travellers[2]; // number of cars on the road heading each direction
// (one is zero at any time)

Car(direction direction) {
	int reverseDirection = !direction;
	P(mutex);
	while (travellers[reverseDirection]) { // if there are travelers coming from the other direction
		blockedCount[direction]++; 
		V(mutex);
		P(blocked[direction]);
		P(mutex);
	}
	// free to go across
	V(mutex);

	// cross the road
	P(mutex);
	travellers[direction]--;
	if (!travellers[direction]) {
		// if it is the last car
		// let the other direction proceed
		while(blockedCount[reverseDirection]--)
			V(blocked[reverseDirection]);
	}
	V(mutex);
}