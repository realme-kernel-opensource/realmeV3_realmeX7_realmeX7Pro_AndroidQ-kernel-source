#ifndef __LOWMEM_DBG_H
#define __LOWMEM_DBG_H

#ifdef CONFIG_OPLUS_FEATURE_LOWMEM_DBG

void oppo_lowmem_dbg(void );

#ifndef CONFIG_MTK_ION
inline int oppo_is_dma_buf_file(struct file *file);
#endif /* CONFIG_MTK_ION */

#else

#ifndef CONFIG_MTK_ION
inline void oppo_lowmem_dbg(void )
{
}
#endif /* CONFIG_MTK_ION */

inline int oppo_is_dma_buf_file(struct file *file)
{
	return 0;
}

#endif /* CONFIG_OPPO_LOWMEM_DBG */

#endif /* __LOWMEM_DBG_H */
