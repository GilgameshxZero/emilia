import { registerComponent } from "../component.js";
import "./arx.js";
import "./coast.js";
import "./flurry.js";
import "./blackfeather.js";

registerComponent(
	`underlay`,
	class extends HTMLElement {
		toCoast() {
			window.history.pushState(null, ``, `/`);
			this.setAttribute(`location`, `coast`);
		}

		toBlackfeather(snapshotName) {
			window.history.pushState(null, ``, `/snapshots/${snapshotName}`);
			this.setAttribute(`location`, `blackfeather`);
			this.shadowRoot
				.querySelector(`emilia-blackfeather`)
				.loadSnapshot(snapshotName);
		}

		toArx() {
			window.history.pushState(null, ``, `/storyworlds`);
			this.setAttribute(`location`, `arx`);
		}
	}
);
