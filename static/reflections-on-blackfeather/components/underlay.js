import { registerComponent } from "../components.js";
import "./arx.js";
import "./coast.js";
import "./flurry.js";
import "./blackfeather.js";

registerComponent(
	`underlay`,
	class extends HTMLElement {
		constructor() {
			super();
		}

		toCoast() {
			this.setAttribute(`location`, `coast`);
		}

		toBlackfeather(snapshotName) {
			this.setAttribute(`location`, `blackfeather`);
			this.shadowRoot
				.querySelector(`emilia-blackfeather`)
				.loadSnapshot(snapshotName);
		}

		toArx() {
			this.setAttribute(`location`, `arx`);
		}
	}
);
