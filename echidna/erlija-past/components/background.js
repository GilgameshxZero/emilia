import { registerComponent } from "../../scripts/component.js";

registerComponent(
	`background`,
	`erlija-past`,
	class extends HTMLElement {
		onDomLoad() {
			this.onScroll = this.onScroll.bind(this);

			// On mobile, and primarily iOS Safari, the background color shows up minutely at the top and bottoms of the screen. We would like it to match the background as closely as possible.
		}

		toTimeline() {
			// hsl-base-1
			document.body.style.backgroundColor = `hsl(227, 50%, 34%)`;
			this.resourceLoad.then(() => {
				this.setAttribute(`lagging`, ``);
				this.setAttribute(`position`, `timeline`);
			});
		}

		toSnapshot() {
			// hsl-base-5
			document.body.style.backgroundColor = `hsl(49, 100%, 73%)`;
			this.resourceLoad.then(() => {
				this.setAttribute(`lagging`, ``);
				this.setAttribute(`position`, `snapshot`);
				this.addEventListener(`transitionend`, () => {
					this.removeAttribute(`lagging`);
				});
			});
		}

		toMap() {
			document.body.style.backgroundColor = `black`;
			this.resourceLoad.then(() => {
				this.setAttribute(`lagging`, ``);
				this.setAttribute(`position`, `map`);
			});
		}

		onScroll() {
			const progress =
				window.scrollY /
				(document.documentElement.scrollHeight -
					document.documentElement.clientHeight);
			this.style.setProperty(`--snapshot-progress`, progress);
		}

		resetScroll() {
			this.style.setProperty(`--snapshot-progress`, 0);
		}
	}
);
