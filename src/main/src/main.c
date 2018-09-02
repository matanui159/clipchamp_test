#include <emscripten.h>
#include "encoder.h"

static void download(int size, const byte_t* data) {
	EM_ASM({
		var size = $0;
		var data = HEAPU8.slice($1, $1 + size);
		var blob = new Blob([data], {type: "octet/stream"});
		var url = window.URL.createObjectURL(blob);
		id("download").href = url;
		id("download").click();
		window.URL.revokeObjectURL(url);
		id("state").innerHTML = "Done!";
	}, size, data);
}

void convert(int width, int height, int fps, int size, byte_t* data) {
	int frame_size = width * height;
	frame_size += frame_size / 2;
	encoder_t* enc = encoder_create(width, height, fps);

	while (size >= frame_size) {
		encoder_frame(enc, data);
		size -= frame_size;
		data += frame_size;
	}

	int out_size;
	const byte_t* out = encoder_data(enc, &out_size);
	download(out_size, out);
	encoder_destroy(enc);
}

int main() {
	EM_ASM(
		id = function(i) {
			return document.getElementById(i);
		};

		id("button").onclick = function(event) {
			id("state").innerHTML = "Reading...";
			id("download").download = id("file").files[0].name + ".264";

			var reader = new FileReader();
			reader.onload = function(event) {
				id("state").innerHTML = "Converting...";
				var array = new Uint8Array(event.target.result);
				ccall("convert", "number", ["number", "number", "number", "number", "array"], [
					id("width").value,
					id("height").value,
					id("fps").value,
					array.byteLength,
					array
				]);
			};
			reader.readAsArrayBuffer(id("file").files[0]);
		};
	);
	return 0;
}