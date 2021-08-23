import component from "../common/component.js";
import "./map.js";

component(
	`splash`,
	class extends HTMLElement {
		// Called from index after load.
		sceneIn() {
			document.body.setAttribute(`active`, ``);
			document.querySelector(`eutopia-sunset`).setAttribute(`scene`, `splash`);
			setTimeout(() => {
				// Transition scene out into map, or another scene based on query parameters.
				document.body.removeAttribute(`active`);

				const queryParams = new URLSearchParams(window.location.search);
				const sceneName = queryParams.get(`scene`) || `map`;
				const scene = document.createElement(`eutopia-${sceneName}`);

				// Attach scene to DOM immediately to begin loading of subcomponents.
				document.body.appendChild(scene);
				document
					.querySelector(`eutopia-sunset`)
					.setAttribute(`scene`, sceneName);

				if (sceneName === `essay`) {
					scene.setEssay(queryParams.get(`essay`));
				}

				setTimeout(() => {
					// replaceChild may cause opacity CSS to not apply for a moment; this is a browser bug.
					document.body.removeChild(this);
					scene.sceneIn();
				}, 2000);
			}, 1500);
		}
	}
);
