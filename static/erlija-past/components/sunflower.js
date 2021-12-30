import { registerComponent } from "../components.js";

// Sunflowers transition between scenes.
registerComponent(
	`sunflower`,
	class extends HTMLElement {
		constructor() {
			super();
			this.onclick = () => {
				document.body.removeAttribute(`active`);
				const sunset = document.querySelector(`emilia-sunset`);
				sunset.setAttribute(`scene`, this.newSceneName);
				const newScene = document.createElement(`emilia-${this.newSceneName}`);
				document.body.appendChild(newScene);
				setTimeout(() => {
					document.body.removeChild(this.oldScene);
					// The scene must have sceneIn to be a valid transition scene for sunflower.
					newScene.sceneIn();
				}, 500);
			};
		}

		// When clicked, sunflowers create a new scene element of newSceneName and replace oldScene with it.
		setTransition(newSceneName, oldScene) {
			this.newSceneName = newSceneName;
			this.oldScene = oldScene;
		}
	}
);
