import { registerComponent } from "../component.js";
import "./feather.js";

registerComponent(
	`flurry`,
	class extends HTMLElement {
		constructor() {
			super();
			this.spawnFeather = this.spawnFeather.bind(this);
		}

		onComponentDOMContentLoaded() {
			this.componentLoad.then(this.spawnFeather);
		}

		spawnFeather() {
			const feather = document.createElement(`emilia-feather`);
			this.shadowRoot.appendChild(feather);
			// Spawn a new feather every so often.
			setTimeout(this.spawnFeather, Math.random() * 300 + 600);
		}
	}
);
