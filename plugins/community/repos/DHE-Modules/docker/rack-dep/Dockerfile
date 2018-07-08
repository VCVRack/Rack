FROM rack:env
ENV VCV_RACK_DIR=/build/Rack
RUN mkdir -p "${VCV_RACK_DIR}" \
  && git clone -n https://github.com/VCVRack/Rack.git "${VCV_RACK_DIR}" || true \
  && cd "${VCV_RACK_DIR}" \
  && git checkout master \
  && git pull \
  && git submodule update --init --recursive \
  && make dep > /dev/null
