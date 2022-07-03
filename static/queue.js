export class Queue {
	#rear;
	#fore;

	constructor() {
		this.#rear = [];
		this.#fore = [];
	}

	// Moves all elements from #rear to #fore. Must only be called when #fore is empty.
	#shift() {
		if (this.#fore.length === 0) {
			while (this.#rear.length !== 0) {
				this.#fore.push(this.#rear.pop());
			}
		}
	}

	size() {
		return this.#fore.length + this.#rear.length;
	}

	push(element) {
		this.#rear.push(element);
	}

	pop() {
		this.#shift();
		return this.#fore.pop();
	}

	front() {
		this.#shift();
		return this.#fore[this.#fore.length - 1];
	}

	back() {
		if (this.#rear.length == 0) {
			return this.#fore[0];
		}
		return this.#rear[this.#rear.length - 1];
	}

	// Iterates through all elements, in order.
	forEach(callable) {
		for (let i = this.#fore.length - 1; i >= 0; i--) {
			callable(this.#fore[i], this.#fore.length - 1 - i);
		}
		for (let i = 0; i < this.#rear.length; i++) {
			callable(this.#rear[i], this.#fore.length + i);
		}
	}
}
