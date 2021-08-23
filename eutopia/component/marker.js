import component from "../common/component.js";
import "./essay.js";

component(
	`marker`,
	class extends HTMLElement {
		constructor() {
			super();
			this.onclick = this.sceneOut;
		}

		// Transition out map scene into essay scene.
		sceneOut() {
			document.body.removeAttribute(`active`);
			document.querySelector(`eutopia-sunset`).setAttribute(`scene`, `essay`);
			const essay = document.createElement(`eutopia-essay`);
			document.body.appendChild(essay);
			essay.setEssay(this.getAttribute(`name`));
			setTimeout(() => {
				document.body.removeChild(document.querySelector(`eutopia-map`));
				essay.sceneIn();
			}, 1000);
		}
	}
);
