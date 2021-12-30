import { registerComponent } from "../components.js";
import { createSceneFromUrl } from "../url.js";
import "./map.js";

registerComponent(
	`splash`,
	class extends HTMLElement {
		// Called from index after load.
		sceneIn() {
			// Randomly select a splash text.
			let texts = this.shadowRoot.querySelectorAll(`h2`);
			texts
				.item(Math.floor(texts.length * Math.random()))
				.setAttribute(`visible`, ``);

			document.body.setAttribute(`active`, ``);
			document.querySelector(`emilia-sunset`).setAttribute(`scene`, `splash`);

			// Clicks or keystrokes transition out the scene.
			this.sceneOut = this.sceneOut.bind(this);
			[`click`, `keydown`, `mousewheel`].forEach((event) => {
				document.addEventListener(event, this.sceneOut);
			});
		}

		sceneOut() {
			[`click`, `keydown`, `mousewheel`].forEach((event) => {
				document.removeEventListener(event, this.sceneOut);
			});

			// Transition scene out into map, or another scene based on query parameters.
			document.body.removeAttribute(`active`);
			const newScene = createSceneFromUrl(window.location.search);
			setTimeout(() => {
				// replaceChild may cause opacity CSS to not apply for a moment; this is a browser bug.
				document.body.removeChild(this);
				newScene.sceneIn();
			}, 500);
		}
	}
);
