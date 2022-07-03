import { registerComponent } from "../component.js";
import { Queue } from "../queue.js";

registerComponent(
	`ping`,
	class extends HTMLElement {
		constructor(history_length = 1024, ping_interval_ms = 1000) {
			super();
			this.history_length = history_length;
			this.ping_interval_ms = ping_interval_ms;
		}

		onComponentDOMContentLoaded() {
			this.componentLoad.then(() => {
				this.sendPing = this.sendPing.bind(this);
				this.history = new Queue();
				this.display = this.shadowRoot.querySelector(`div > div`);
				this.sendPing();
			});
		}

		onPing(ping) {
			this.history.push(ping);
			if (this.history.size() > this.history_length) {
				this.history.pop();
			}
			this.display.innerText = `${this.history.back()}ms`;
		}

		sendPing() {
			const timeSent = Date.now();
			fetch(`/api/ping`)
				.then(() => {
					this.onPing(Date.now() - timeSent);
				})
				.catch(() => {
					this.onPing(this.ping_interval_ms);
				});
			setTimeout(this.sendPing, this.ping_interval_ms);
		}
	}
);
