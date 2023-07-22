ARG base_tag=latest
FROM ghcr.io/testillano/queuedispatcher_builder:${base_tag}
MAINTAINER testillano

LABEL testillano.queuedispatcher.description="ert_queuedispatcher library image"

WORKDIR /code/build

ARG make_procs=4
ARG build_type=Release

# ert_queuedispatcher
COPY . /code/build/queuedispatcher/
RUN set -x && \
    cd queuedispatcher && cmake -DCMAKE_BUILD_TYPE=${build_type} . && make -j${make_procs} && make install && \
    cd .. && rm -rf queuedispatcher && \
    set +x

