FROM rack:env
ARG RACK_SDK_VERSION
RUN curl -o rack-sdk.zip https://vcvrack.com/downloads/Rack-SDK-${RACK_SDK_VERSION}.zip \
    && unzip rack-sdk.zip
VOLUME /dist
CMD bash
