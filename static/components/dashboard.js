import { registerComponent } from "../component.js";
import "./ping.js";

registerComponent(
	`dashboard`,
	class extends HTMLElement {
		componentIn() {
			this.componentLoad.then(() => {});
		}
	}
);
