import component from "../common/component.js";
import "./marker.js";
import "./sunflower.js";
import "./erlija.js";

component(
	`map`,
	class extends HTMLElement {
		constructor() {
			super();
		}

		// Transitions in map scene; transition out by markers.
		sceneIn() {
			this.componentLoad.then(() => {
				const sunflower = this.shadowRoot.querySelector(`eutopia-sunflower`);
				Promise.all([
					sunflower.componentLoad,
					...Array.from(this.shadowRoot.querySelectorAll(`eutopia-marker`)).map(
						(marker) => {
							return marker.componentLoad;
						}
					)
				]).then(() => {
					document.body.setAttribute(`active`, ``);
					sunflower.setTransition(`erlija`, this);

					window.history.pushState(null, ``, window.location.pathname);
				});
			});
		}
	}
);
