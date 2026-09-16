#include "ffmpegfilter.h"
#include "ffmpeghelper.h"
#include "filterhelper.h"

QString FFmpegFilter::getFilter(const FilterData &filterData)
{
    //滤镜内容字符串集合
    QStringList listFilter;
    bool hardware = (filterData.formatIn == AV_PIX_FMT_NV12);
    if (filterData.rotate != 0) {
        QString filter = FilterHelper::getFilter(filterData.rotate, hardware);
        listFilter << filter;
    }

    //加入标签信息
    foreach (OsdInfo osd, filterData.listOsd) {
        QString filter = FilterHelper::getFilter(osd, hardware);
        if (!filter.isEmpty()) {
            listFilter << filter;
        }
    }

    //加入图形信息
    foreach (GraphInfo graph, filterData.listGraph) {
        QString filter = FilterHelper::getFilter(graph, hardware);
        if (!filter.isEmpty()) {
            listFilter << filter;
        }
    }

    //加入其他滤镜
    QString filter = FilterHelper::getFilter();
    if (!filter.isEmpty()) {
        listFilter << filter;
    }

    return listFilter.join(",");
}

int FFmpegFilter::initFilter(AbstractVideoThread *thread, AVStream *stream, AVCodecContext *avctx, FilterData &filterData)
{
    int result = -1;
    if (!filterData.enable) {
        return result;
    }

    //貌似硬解码不支持滤镜
    if (filterData.formatIn == AV_PIX_FMT_NV12) {
        return result;
    }

    //先释放相关资源
    freeFilter(filterData);

    //获取滤镜字符串
    QString filters = getFilter(filterData);
    if (filters.isEmpty()) {
        return result;
    }

    //输入帧序列的参数信息
    QStringList listArg;
    listArg << QString("video_size=%1x%2").arg(avctx->width).arg(avctx->height);
    listArg << QString("pix_fmt=%1").arg(avctx->pix_fmt);
    listArg << QString("time_base=%1/%2").arg(stream->time_base.num).arg(stream->time_base.den);
    listArg << QString("pixel_aspect=%1/%2").arg(avctx->sample_aspect_ratio.num).arg(avctx->sample_aspect_ratio.den);
    QString args = listArg.join(":");

    //输入帧格式
    enum AVPixelFormat pix_fmts[] = {filterData.formatIn, AV_PIX_FMT_NONE};

    //获取要使用的滤镜
    const AVFilter *filterSrc = avfilter_get_by_name("buffer");
    const AVFilter *filterSink = avfilter_get_by_name("buffersink");

    //创建输入输出滤镜参数
    AVFilterInOut *inputs = avfilter_inout_alloc();
    AVFilterInOut *outputs = avfilter_inout_alloc();

    //创建滤镜容器
    filterData.filterGraph = avfilter_graph_alloc();
    if (!inputs || !outputs || !filterData.filterGraph) {
        result = AVERROR(ENOMEM);
        goto end;
    }

    //创建输入滤镜
    result = avfilter_graph_create_filter(&filterData.filterSrcCtx, filterSrc, "in", args.toUtf8().constData(), NULL, filterData.filterGraph);
    if (result < 0) {
        thread->debug("滤镜处理", QString("创建输入滤镜失败: %1").arg(FFmpegHelper::getError(result)), "");
        goto end;
    }

    //创建输出滤镜
    result = avfilter_graph_create_filter(&filterData.filterSinkCtx, filterSink, "out", NULL, NULL, filterData.filterGraph);
    if (result < 0) {
        thread->debug("滤镜处理", QString("创建输出滤镜失败: %1").arg(FFmpegHelper::getError(result)), "");
        goto end;
    }

    //设置输出滤镜格式
    result = av_opt_set_int_list(filterData.filterSinkCtx, "pix_fmts", pix_fmts, filterData.formatOut, AV_OPT_SEARCH_CHILDREN);
    if (result < 0) {
        thread->debug("滤镜处理", QString("设置输出滤镜格式: %1").arg(FFmpegHelper::getError(result)), "");
        goto end;
    }

    //设置滤镜的参数
    outputs->name = av_strdup("in");
    outputs->filter_ctx = filterData.filterSrcCtx;
    outputs->pad_idx = 0;
    outputs->next = NULL;

    inputs->name = av_strdup("out");
    inputs->filter_ctx = filterData.filterSinkCtx;
    inputs->pad_idx = 0;
    inputs->next = NULL;

    //初始化滤镜
    result = avfilter_graph_parse_ptr(filterData.filterGraph, filters.toUtf8().constData(), &inputs, &outputs, NULL);
    if (result < 0) {
        thread->debug("滤镜处理", QString("初始化滤镜失败: %1").arg(FFmpegHelper::getError(result)), "");
        goto end;
    }

    //应用滤镜配置
    result = avfilter_graph_config(filterData.filterGraph, NULL);
    if (result < 0) {
        thread->debug("滤镜处理", QString("应用滤镜配置失败: %1").arg(FFmpegHelper::getError(result)), "");
        goto end;
    }

end:
    //释放对应的输入输出
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
    filterData.isOk = (result >= 0);
    return result;
}

void FFmpegFilter::freeFilter(FilterData &filterData)
{
    if (filterData.isOk) {
        filterData.enable = true;
        filterData.init = true;
        filterData.isOk = false;

        avfilter_free(filterData.filterSrcCtx);
        avfilter_free(filterData.filterSinkCtx);
        avfilter_graph_free(&filterData.filterGraph);
        filterData.filterSrcCtx = NULL;
        filterData.filterSinkCtx = NULL;
        filterData.filterGraph = NULL;
    }
}
