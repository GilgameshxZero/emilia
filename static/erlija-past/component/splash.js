import component from "../common/component.js";
import sceneFromUri from "../common/uri.js";
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
				const newScene = sceneFromUri(window.location.search);
				setTimeout(() => {
					// replaceChild may cause opacity CSS to not apply for a moment; this is a browser bug.
					document.body.removeChild(this);
					newScene.sceneIn();
				}, 2000);
			}, 1500);
		}
	}
);
