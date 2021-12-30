import { registerComponent } from "../components.js";
import "./snapshot.js";

registerComponent(
	`marker`,
	class extends HTMLElement {
		constructor() {
			super();
			this.onclick = () => {
				const map = document.querySelector(`emilia-map`);
				map.sceneOut.bind(map)(this.getAttribute(`name`));
			};
		}
	}
);
