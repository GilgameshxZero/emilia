let FRAMES_PER_SECOND = 60;
let MS_PER_FRAME = 1000.0 / FRAMES_PER_SECOND;
let GRAVITATIONAL_CONST = 0.3;

var body;
var bubbleElems;

var width;
var height;
var bubbleSize;

var bubbleCenters = [];
var bubbleVels = [];

window.onresize = onResize;

window.onload = function() {
	body = document.getElementsByTagName("BODY")[0];
	bubbleElems = document.getElementsByClassName("bubble");

	while (bubbleCenters.length < bubbleElems.length) {
		bubbleCenters.push([0, 0]);
		bubbleVels.push([0, 0]);
	}

	//move all children of bubbles in a subdiv

	onResize();

	//randomly splash all bubbles onto page
	for (let a = 0;a < bubbleElems.length;a++) {
		var collision = true;
		while (collision) {
			bubbleCenters[a][0] = Math.random() * (width - bubbleSize) + bubbleSize / 2;
			bubbleCenters[a][1] = Math.random() * (height - bubbleSize) + bubbleSize / 2;

			collision = false;
			for (let b = 0;b < a;b++) {
				var relX = bubbleCenters[b][0] - bubbleCenters[a][0];
				var relY = bubbleCenters[b][1] - bubbleCenters[a][1];
				var dist = Math.sqrt(relX * relX + relY * relY);

				if (dist <= bubbleSize) {
					collision = true;
					break;
				}
			}
		}

		bubbleElems[a].style.left = bubbleCenters[a][0] - bubbleSize / 2 + "px";
		bubbleElems[a].style.top = bubbleCenters[a][1] - bubbleSize / 2 + "px";
	}

	updateSystem();
}

function onResize() {
	width = window.getComputedStyle(body, null).getPropertyValue("width").slice(0, -2);
	height = window.getComputedStyle(body, null).getPropertyValue("height").slice(0, -2);
	bubbleSize = Math.min(width, height) / Math.sqrt(bubbleElems.length) / 2;

	for (let a = 0;a < bubbleElems.length;a++) {
		bubbleElems[a].style.width = bubbleElems[a].style.height = bubbleSize + "px";
		bubbleElems[a].style.borderRadius = bubbleSize / 2 + "px";

		bubbleElems[a].style.left = bubbleCenters[a][0] - bubbleSize / 2 + "px";
		bubbleElems[a].style.top = bubbleCenters[a][1] - bubbleSize / 2 + "px";
	}
}

function updateSystem() {
	if (typeof  updateSystem.lastTime == "undefined") {
		updateSystem.lastTime = 0;
	} 

	let currentTime = new Date().getTime();
	let diff = updateSystem.lastTime == 0 ? 1 : currentTime - updateSystem.lastTime;

	//each object exerts a force on each other
	let bubbleMass = 1;

	let elasticity = 0.7;

	for (let a = 0;a < bubbleCenters.length;a++) {
		var acc = [0, 0];
		var dist;
		for (let b = 0;b < bubbleCenters.length;b++) {
			if (a == b) {
				continue;
			}
			var relX = bubbleCenters[b][0] - bubbleCenters[a][0];
			var relY = bubbleCenters[b][1] - bubbleCenters[a][1];
			var relAng = Math.atan2(relY, relX);
			dist = Math.sqrt(relX * relX + relY * relY);

			if (dist <= bubbleSize) {
				//collision, conservation of momentum up to a degree
				var velA = Math.sqrt(bubbleVels[a][0] * bubbleVels[a][0] + bubbleVels[a][1] * bubbleVels[a][1]);
				var velB = Math.sqrt(bubbleVels[b][0] * bubbleVels[b][0] + bubbleVels[b][1] * bubbleVels[b][1]);
				var tMomentumX = bubbleMass * (bubbleVels[a][0] + bubbleVels[b][0]) * elasticity;
				var tMomentumY = bubbleMass * (bubbleVels[a][1] + bubbleVels[b][1]) * elasticity;

				//modify velocity to reflect momentum changes
				//bubbleVels[a][0] = tMomentumX / 2 / bubbleMass;
			}

			var hypAcc = GRAVITATIONAL_CONST * bubbleMass * bubbleMass / dist / dist;

			//decompose into components
			acc[0] += Math.cos(relAng) * hypAcc;
			acc[1] += Math.sin(relAng) * hypAcc;
		}

		//universe boundaries are inelastic collisions

		//apply force to vel
		bubbleVels[a][0] += diff * acc[0];
		bubbleVels[a][1] += diff * acc[1];
		
		//apply vel to pos
		bubbleCenters[a][0] += diff * bubbleVels[a][0];
		bubbleCenters[a][1] += diff * bubbleVels[a][1];

		//update positions
		bubbleElems[a].style.left = bubbleCenters[a][0] - bubbleSize / 2 + "px";
		bubbleElems[a].style.top = bubbleCenters[a][1] - bubbleSize / 2 + "px";
	}

	updateSystem.lastTime = new Date().getTime();
	setTimeout(updateSystem, MS_PER_FRAME - (updateSystem.lastTime - currentTime));
}