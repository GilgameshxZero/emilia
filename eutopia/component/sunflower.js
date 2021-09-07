import component from "../common/component.js";

// Sunflowers transition between scenes.
component(
	`sunflower`,
	class extends HTMLElement {
		constructor() {
			super();
			this.onclick = () => {
				document.body.removeAttribute(`active`);
				const sunset = document.querySelector(`eutopia-sunset`);
				sunset.style.removeProperty(`--progress`);
				sunset.setAttribute(`scene`, this.newSceneName);
				const newScene = document.createElement(`eutopia-${this.newSceneName}`);
				document.body.appendChild(newScene);
				setTimeout(() => {
					document.body.removeChild(this.oldScene);
					// The scene must have sceneIn to be a valid transition scene for sunflower.
					newScene.sceneIn();
				}, 1000);
			};
		}

		// When clicked, sunflowers create a new scene element of newSceneName and replace oldScene with it.
		setTransition(newSceneName, oldScene) {
			this.newSceneName = newSceneName;
			this.oldScene = oldScene;
		}
	}
);
